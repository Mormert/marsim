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

Mqtt::Mqtt() { init(); }

Mqtt::~Mqtt() { cleanup(); }

void
Mqtt::connectMqtt(const std::string &address, int port)
{
    // TODO: Connect to MQTT Broker
    // Set isConnected bool to true IF we connected successfully
}

void
Mqtt::disconnectMqtt()
{
    // TODO: Disconnect from MQTT

    is_connected = false;
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
}

void
Mqtt::init()
{
    // TODO: Initialize the MQTT library
}

void
Mqtt::cleanup()
{
    // TODO: Cleanup the MQTT library
}
