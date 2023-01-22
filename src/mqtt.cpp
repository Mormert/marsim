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

#include <chrono>

#include <json.hpp>


void on_PNGmessage(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message){
    printf("Image File Received. \n");
    if(message->payloadlen){
        //if(message->topic == "map/lunar/sr_to_sim")
        //{
        std::ofstream image_file("data/lunar_received.png", std::ios::binary);
        image_file.write(reinterpret_cast<const char*>(message->payload), message->payloadlen);
        image_file.close();
        std::cout << "Image received and saved as lunar_image.png" << std::endl;
        //}
        //printf("%s %s\n", message->topic, message->payload);
    }else{
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}


//called on received message for commands
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    printf("MESSAGE RECEIVED!!!!!!!!!!!!!!!!!! \n");
    if(message->payloadlen){
        printf("%s %s\n", message->topic, message->payload);
    }else{
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}
//shows if connected correctly
void on_connect(struct mosquitto *mosq, void *userdata, int result)
{
    printf("connecting...\n");
    if(!result){
        printf("connected successfully");
        /* Connected successfully. */

    }else{
        /* Connect failed. */

    }
}
//log to debug in case of error during connect/pub/sub
void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str) {
    printf("%s\n", str);
}


Mqtt::Mqtt() {
    init();
}

Mqtt::~Mqtt() { cleanup(); }

void
Mqtt::connectMqtt(const std::string &address, int port)
{
    // TODO: Connect to MQTT Broker
    // Set isConnected bool to true IF we connected successfully
    if(mosquitto_connect(mqtt, address.c_str(), port, 60)){
        is_connected = false;
        std::cout << "could not connect!" << std::endl;
    }else{
        is_connected = true;
    }
}

void
Mqtt::disconnectMqtt()
{
    // TODO: Disconnect from MQTT
    if(mosquitto_disconnect(mqtt) == MOSQ_ERR_SUCCESS){
        is_connected = false;

    }else{
        std::cout << "Failed to disconnect! try again..." << std::endl;
    }


}

void
Mqtt::send(const std::string &topic, const std::string &message_type, const std::string &payload)
{
    if (!is_connected) {
        return;
    }

    nlohmann::json j;

    j["type"] = message_type;

    const auto tp = std::chrono::system_clock::now();
    j["time"] = tp.time_since_epoch().count();

    j["data"] = payload;

    sendMqtt(topic, j.dump());
}

void
Mqtt::processMqtt()
{
    if(!is_connected)
    {
        return;
    }

    // TODO: Process MQTT networking.
    // Call mosquitto_loop() here.
    mosquitto_loop(mqtt, 0, 1);
}

bool
Mqtt::isConnected()
{
    return is_connected;
}

void
Mqtt::sendMqtt(const std::string &topic, const std::string &data)
{
    // TODO: Send message with MQTT.
    //mosquitto_subscribe(mqtt, NULL, "map/lunar/sr_to_sim", 1);
    mosquitto_publish(mqtt, NULL, topic.c_str(), data.length(), data.c_str(), 1, false);//}
}

void
Mqtt::init()
{
    // TODO: Initialize the MQTT library
    mosquitto_lib_init();
    mqtt = mosquitto_new("Simulator_Channel", true, NULL);
    mosquitto_username_pw_set(mqtt, "simtor", "simtor23");
    // set the path to the certificate and key files
    int rt = mosquitto_tls_set(mqtt, "data/cacert.pem", NULL, NULL, NULL, NULL);
    if( rt == MOSQ_ERR_SUCCESS){ std::cout << "certificate accepted" << std::endl;}
    mosquitto_tls_opts_set(mqtt, 1, "tlsv1.2", NULL);
    mosquitto_tls_insecure_set(mqtt, true);
    mosquitto_log_callback_set(mqtt, my_log_callback);
    mosquitto_connect_callback_set(mqtt, on_connect);
    mosquitto_message_callback_set(mqtt, on_message); //change this to on_PNGmessage when receiving the image from the situation reporting module
}

void
Mqtt::cleanup()
{
    // TODO: Cleanup the MQTT library
    mosquitto_destroy(mqtt);
    mosquitto_lib_cleanup();
}
