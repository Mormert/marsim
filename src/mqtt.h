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

#ifndef MARSIM_MQTT_H
#define MARSIM_MQTT_H

#include <mosquitto.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include <unordered_map>

#include <mosquitto.h>
#include <json.hpp>

class Simulation;

class Mqtt
{
public:
    Mqtt();

    ~Mqtt();

    void connectMqtt(const std::string &address, int port);

    void disconnectMqtt();

    void send(const std::string &topic, const std::string &message_type, const nlohmann::json &payload);

    void processMqtt(int32_t step);

    void setSimulationPtr(Simulation* sim){this->simulation = sim;}

    static Mqtt &
    getInstance()
    {
        static Mqtt instance;
        return instance;
    }

    bool isConnected();

    bool *useMessagePackBool();

    int* getCompressionInt();

    float getEmissionSpeed();

    unsigned int getSentBytes();

    unsigned int getMessagesSent();

    static void receiveMsgMotors(const nlohmann::json & data);
    static void receiveMsgPickup(const nlohmann::json & data);
    static void receiveMsgDrop(const nlohmann::json & data);
    static void receiveMsgLaserAngle(const nlohmann::json & data);
    static void receiveMsgLaserShoot(const nlohmann::json & data);

    static void receiveMsgRequestImage(const nlohmann::json & data);

private:
    // Publishes the payload for the given topic
    void sendMqtt(const std::string &topic, const std::string &data);

    void sendQueuedMessages();

    // Topic, Message
    std::unordered_map<std::string, std::vector<nlohmann::json>> queuedMessages;

    void init();

    void cleanup();

    bool is_connected = false;



    unsigned int sentMessages{0};
    unsigned int sentBytesTotal{0};
    unsigned int sentBytesSecond{0};
    unsigned int sentBytesLastSecond{0};

    int compression{0}; // 0 = no, 1 = gzip, 2 = zlib
    bool use_messagepack{false};

    mosquitto *mqtt;
    Simulation* simulation;
};

#endif // MARSIM_MQTT_H
