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

#include "temperature_sensor.h"
#include "framework/draw.h"
#include "glm/common.hpp"
#include "json.hpp"
#include "mqtt.h"
#include "simulation.h"
#include "volcano.h"

TemperatureSensor::TemperatureSensor(Simulation *simulation, b2Vec2 pos) : PhysicalWeatherSensor(simulation, pos)
{
    name = "Temperature Sensor";
}

void
TemperatureSensor::update()
{
    g_debugDraw.DrawCircle(getPosition(), 1.f, b2Color{1.f, 0.f, 0.f, 1.f});

    auto &&tornadoes = simulation->GetTornados();
    auto &&volcanoes = simulation->GetVolcanoes();

    float temperature = 25.f;
    constexpr float tornadoTemperature = 5.f;
    float volcanoTemperature = 100.f;

    if (simulation->volcano->isActive()) {
        volcanoTemperature = 850.f;
    }

    for (auto &&tornado : tornadoes) {
        b2Vec2 strength{tornado.pos - getPosition()};
        float attenuation = strength.LengthSquared();
        attenuation /= 1.f;
        attenuation = glm::clamp(attenuation, 0.f, 100000.f);
        temperature = glm::mix(tornadoTemperature, temperature, attenuation / 100000.f);
    }

    for (auto &&volcano : volcanoes) {
        b2Vec2 strength{volcano.pos - getPosition()};
        float attenuation = strength.LengthSquared();
        attenuation /= 1.f;
        attenuation = glm::clamp(attenuation, 0.f, 100000.f);
        temperature = glm::mix(volcanoTemperature, temperature, attenuation / 100000.f);
    }

    std::string str = std::to_string(temperature) + "'C";
    g_debugDraw.DrawString(getPosition(), str.c_str());

    if (updateCounter % updateFrequency == 0) {
        nlohmann::json j;

        auto pos = getPosition();
        j["pos"] = {{"x", pos.x}, {"y", pos.y}};

        j["temp"] = temperature;
        j["id"] = object_id;

        Mqtt::getInstance().send("sim/out/sensors", name, j);
    }

    updateCounter++;
}
