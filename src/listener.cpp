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

#include <iostream>
#include <sstream>

#include <json.hpp>
#include <mosquitto.h>
#include <zlc/zlibcomplete.hpp>

bool isConnected = false;

int recvMessages = 0;
int recvBytesTotal = 0;

int useCompression = 0;
int useMessagepack = 0;

// shows if connected correctly
void
on_connect_listener(struct mosquitto *mosq, void *userdata, int result)
{
    printf("Connecting...\n");
    if (!result) {
        std::cout << "Connection succeeded!" << std::endl;
        isConnected = true;
    } else {
        std::cerr << "CONNECTION FAILED!" << std::endl;
    }
}

void
on_message_listener(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    recvMessages++;
    recvBytesTotal += message->payloadlen;

    if (message->payloadlen) {
        printf("Received MQTT message topic(%s):", message->topic);

        std::string payloadStr((char *)message->payload, message->payloadlen);

        // Decompress
        if (useCompression == 2) {
            zlibcomplete::GZipDecompressor gZipDecompressor;
            payloadStr = gZipDecompressor.decompress(payloadStr);
        } else if (useCompression == 1) {
            zlibcomplete::ZLibDecompressor zLibDecompressor{};
            payloadStr = zLibDecompressor.decompress(payloadStr);
        }

        try {
            nlohmann::json j;
            if (useMessagepack) {
                j = nlohmann::json::from_msgpack(payloadStr);
            } else {
                j = nlohmann::json::parse(payloadStr);
            }
            printf("%s\n", j.dump(4).c_str());
        } catch (std::exception e) {
            printf("%s\n", (char *)message->payload);
        }

    }else {
        printf("%s (null)\n", message->topic);
    }
    fflush(stdout);
}

void
my_log_callback_listener(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
    printf("%s\n", str);
}

int main(){

    std::string address;
    std::cout << "Enter the address key (like for example bd86ad7d91574abab844ada07ad73b31.s1.eu.hivemq.cloud)" << std::endl;
    std::cin >> address;

    std::cout << "Enter the address port (like for example 8883)" << std::endl;
    int port;
    std::cin >> port;

    std::vector<std::string> topics;
    std::cout << "Enter topics to subscribe to, like this pattern: sim/out/general;sim/out/image" << std::endl;

    std::string input, line;
    std::cin >> input;
    std::stringstream sinput(input);

    while (std::getline(sinput, line, ';'))
        topics.push_back(line);

    for(auto& topic : topics)
    {
        std::cout << "Will subscribe to " << topic << std::endl;
    }

    std::cout << "Use MESSAGEPACK decoding? Enter 1 for yes, 0 for no: ";
    std::cin >> useMessagepack;

    std::cout << "Use ZLIB decoding? Enter 1 for yes, 0 for no: ";
    std::cin >> useCompression;


    mosquitto *mqtt;

    mosquitto_lib_init();
    mqtt = mosquitto_new("Simulator_Channel_Listener", true, NULL);
    mosquitto_username_pw_set(mqtt, "simtor", "simtor23");
    // set the path to the certificate and key files
    int rt = mosquitto_tls_set(mqtt, "data/cacert.pem", NULL, NULL, NULL, NULL);
    if (rt == MOSQ_ERR_SUCCESS) {
        std::cout << "Certificate accepted!" << std::endl;
    }
    mosquitto_tls_opts_set(mqtt, 1, "tlsv1.2", NULL);
    mosquitto_tls_insecure_set(mqtt, true);

    mosquitto_log_callback_set(mqtt, my_log_callback_listener);
    mosquitto_connect_callback_set(mqtt, on_connect_listener);
    mosquitto_message_callback_set(
        mqtt, on_message_listener);

    if (mosquitto_connect(mqtt, address.c_str(), port, 60)) {
        isConnected = false;
        std::cout << "Could not connect!" << std::endl;
    } else {
        const auto subToTopic = [&](const std::string& topic){
            if (mosquitto_subscribe(mqtt, NULL, topic.c_str(), 0) != MOSQ_ERR_SUCCESS) {
                std::cerr << "Failed to subscribe!" << std::endl;
            }
        };

        for(auto topic : topics){
            subToTopic(topic);
        }
    }

    while(true){
        mosquitto_loop(mqtt, 0, 1);
    }

    std::cout << "Exiting";
    return 0;
}