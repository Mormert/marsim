// MIT License

// Copyright (c) 2023 Johan Lind, Ermias Tewolde

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "mqtt.h"

#include "robot.h"
#include "simulation.h"
#include "wheel.h"
#include <chrono>
#include <fstream>
#include <sstream>

#include <json.hpp>
#include <zlc/zlibcomplete.hpp>

void
on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    Mqtt::getInstance().receivedMessages++;
    Mqtt::getInstance().receivedBytesTotal += message->payloadlen;
    Mqtt::getInstance().receivedBytesSecond += message->payloadlen;
    if (message->payloadlen) {
        if (strcmp(message->topic, "sim/in/image") == 0) {
            std::ofstream image_file("data/lunar_received.png", std::ios::binary);
            image_file.write(reinterpret_cast<const char *>(message->payload), message->payloadlen);
            image_file.close();
            // call function for update of image here
            std::cout << "Image received and saved as lunar_image.png" << std::endl;
        }
        if (strcmp(message->topic, "sim/control") == 0) {
            // TO DO: receive commands through this

            std::string payloadStr((char *)message->payload, message->payloadlen);

            try {
                nlohmann::json j = nlohmann::json::parse(payloadStr);
                std::string type = j["type"];
                nlohmann::json jsonPayload = j["data"];

                if (type == "motors") {
                    Mqtt::receiveMsgMotors(jsonPayload);
                } else if (type == "pickup") {
                    Mqtt::receiveMsgPickup(jsonPayload);
                } else if (type == "drop") {
                    Mqtt::receiveMsgDrop(jsonPayload);
                } else if (type == "shoot_laser") {
                    Mqtt::receiveMsgLaserShoot(jsonPayload);
                } else if (type == "laser_angle") {
                    Mqtt::receiveMsgLaserAngle(jsonPayload);
                } else if (type == "request_satellite_image") {
                    Mqtt::receiveMsgRequestImage(jsonPayload);
                }

            } catch (std::exception e) {
                std::cout << "Something went wrong trying to interpret message: " << e.what() << std::endl;
            }

        } else {
            // the message is not the image on the image channel, therefore...
            printf("%s %s\n", message->topic, message->payload);
        }

        //}
        // printf("%s %s\n", message->topic, message->payload);
    } else {
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

// called on received message
void
on_message_(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    printf("MESSAGE RECEIVED!!!!!!!!!!!!!!!!!! \n");
    if (message->payloadlen) {
        printf("%s %s\n", message->topic, message->payload);
    } else {
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}
// shows if connected correctly
void
on_connect(struct mosquitto *mosq, void *userdata, int result)
{
    printf("connecting...\n");
    if (!result) {
        printf("connected successfully");
        /* Connected successfully. */

    } else {
        /* Connect failed. */
    }
}
// log to debug in case of error during connect/pub/sub
void
my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    printf("%s\n", str);
}

Mqtt::Mqtt() { init(); }

Mqtt::~Mqtt() { cleanup(); }

void
Mqtt::connectMqtt(const std::string &address, int port)
{
    // TODO: Connect to MQTT Broker
    // Set isConnected bool to true IF we connected successfully
    if (mosquitto_connect(mqtt, address.c_str(), port, 60)) {
        is_connected = false;
        std::cout << "could not connect!" << std::endl;
    } else {
        is_connected = true;
        if (mosquitto_subscribe(mqtt, NULL, "sim/control", 1) != MOSQ_ERR_SUCCESS) {
            std::cerr << "Failed to subscribe!" << std::endl;
        }
        if (mosquitto_subscribe(mqtt, NULL, "sim/image", 1) != MOSQ_ERR_SUCCESS) {
            std::cerr << "Failed to subscribe!" << std::endl;
        }
    }
}

void
Mqtt::disconnectMqtt()
{
    if (mosquitto_disconnect(mqtt) == MOSQ_ERR_SUCCESS) {
        is_connected = false;

    } else {
        std::cout << "Failed to disconnect! try again..." << std::endl;
    }
}

void
Mqtt::send(const std::string &topic, const std::string &message_type, const nlohmann::json &payload)
{
    if (!is_connected) {
        return;
    }

    nlohmann::json j;
    j["type"] = message_type;
    j["data"] = payload;

    auto &&vec = queuedMessages[topic];
    vec.push_back(j);
}

void
Mqtt::processMqtt(int32_t step)
{
    if (!is_connected) {
        return;
    }

    if (step % 60 == 0) {
        sentBytesLastSecond = sentBytesSecond;
        sentBytesSecond = 0;
        receivedBytesLastSecond = receivedBytesSecond;
        receivedBytesSecond = 0;
    }

    if (step % 3 == 0) {
        // We send the actual messages at a frequency of 20 Hz
        sendQueuedMessages();
    }

    mosquitto_loop(mqtt, 0, 1);
}

void
Mqtt::sendQueuedMessages()
{
    const auto tp = std::chrono::system_clock::now();

    // Send all messages as a batch for each topic
    for (auto &&[topic, msgs] : queuedMessages) {

        nlohmann::json j;
        j["time"] = tp.time_since_epoch().count();
        j["msgs"] = msgs;

        std::string jsonString;

//        std::cout << j.dump(4) << std::endl;

        if (use_messagepack) {
            auto msgPack = nlohmann::json::to_msgpack(j);
            jsonString = std::string(msgPack.begin(), msgPack.end());
        } else {
            jsonString = j.dump();
        }

        if (compression == 1) {
            zlibcomplete::GZipCompressor gZipCompressor(9, zlibcomplete::flush_parameter::auto_flush);
            jsonString = gZipCompressor.compress(jsonString);
            gZipCompressor.finish();
        } else if (compression == 2) {
            zlibcomplete::ZLibCompressor zLibCompressor(9, zlibcomplete::flush_parameter::auto_flush);
            jsonString = zLibCompressor.compress(jsonString);
            zLibCompressor.finish();
        }

        sendMqtt(topic, jsonString);
    }

    queuedMessages = {};
}

bool
Mqtt::isConnected()
{
    return is_connected;
}

void
Mqtt::sendMqtt(const std::string &topic, const std::string &data)
{
    mosquitto_publish(mqtt, NULL, topic.c_str(), data.length(), data.c_str(), 0, false);
    sentBytesTotal += data.length();
    sentBytesSecond += data.length();
    sentMessages++;

    std::cout << "SENT MSG WITH SIZE: " << data.length() << std::endl;
}

void
Mqtt::init()
{
    mosquitto_lib_init();
    mqtt = mosquitto_new("Simulator_Channel", true, NULL);
    mosquitto_username_pw_set(mqtt, "simtor", "simtor23");
    // set the path to the certificate and key files
    int rt = mosquitto_tls_set(mqtt, "data/cacert.pem", NULL, NULL, NULL, NULL);
    if (rt == MOSQ_ERR_SUCCESS) {
        std::cout << "Certificate accepted!" << std::endl;
    }
    mosquitto_tls_opts_set(mqtt, 1, "tlsv1.2", NULL);
    mosquitto_tls_insecure_set(mqtt, true);
    //mosquitto_log_callback_set(mqtt, my_log_callback);
    //mosquitto_connect_callback_set(mqtt, on_connect);
    mosquitto_message_callback_set(
        mqtt, on_message); // change this to on_PNGmessage when receiving the image from the situation reporting module
}

void
Mqtt::cleanup()
{
    mosquitto_destroy(mqtt);
    mosquitto_lib_cleanup();
}

bool *
Mqtt::useMessagePackBool()
{
    return &use_messagepack;
}
unsigned int
Mqtt::getSentBytes()
{
    return sentBytesTotal;
}
int *
Mqtt::getCompressionInt()
{
    return &compression;
}
float
Mqtt::getEmissionSpeed()
{
    return sentBytesLastSecond;
}
unsigned int
Mqtt::getMessagesSent()
{
    return sentMessages;
}

void
Mqtt::receiveMsgPickup(const nlohmann::json &data)
{
    Mqtt::getInstance().simulation->GetRobot()->pickup();
}

void
Mqtt::receiveMsgMotors(const nlohmann::json &data)
{
    try {
        float left = data["left"];
        float right = data["right"];

        Mqtt::getInstance().simulation->GetRobot()->leftAccelerate = left;
        Mqtt::getInstance().simulation->GetRobot()->rightAccelerate = right;
    } catch (std::exception &e) {
        std::cerr << "Failed to set motor speed: " << e.what() << std::endl;
    }
}

void
Mqtt::receiveMsgDrop(const nlohmann::json &data)
{
    try {
        std::string item = data["item"];

        if (!Mqtt::getInstance().simulation->GetRobot()->drop(item)) {
            std::cerr << "Failed to drop item: " << item << ", it does not exist in storage." << std::endl;
        }
    } catch (std::exception &e) {
        std::cerr << "Failed to drop item: " << e.what() << std::endl;
    }
}

void
Mqtt::receiveMsgLaserAngle(const nlohmann::json &data)
{
    try {
        float deg = data["angle"];
        Mqtt::getInstance().simulation->GetRobot()->setLaserAngleDegrees(deg);
    } catch (std::exception &e) {
        std::cerr << "Failed to set laser angle: " << e.what() << std::endl;
    }
}

void
Mqtt::receiveMsgLaserShoot(const nlohmann::json &data)
{
    Mqtt::getInstance().simulation->GetRobot()->shootLaser();
    std::cout << "Firing laser..." << std::endl;
}

void
Mqtt::receiveMsgRequestImage(const nlohmann::json &data)
{
    std::ifstream image_file("data/lunar_image.png", std::ios::binary);
    std::vector<unsigned char> image_data((std::istreambuf_iterator<char>(image_file)),
                                          std::istreambuf_iterator<char>());
    mosquitto_publish(Mqtt::getInstance().mqtt, NULL, "sim/out/image", image_data.size(), image_data.data(), 1, false);
}
