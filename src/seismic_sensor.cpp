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

#include "seismic_sensor.h"
#include "framework/draw.h"
#include "glm/common.hpp"
#include "json.hpp"
#include "mqtt.h"
#include "simulation.h"
#include "volcano.h"
#include <random>

SeismicSensor::SeismicSensor(Simulation *simulation, b2Vec2 pos) : PhysicalWeatherSensor(simulation, pos)
{
    name = "Seismic Sensor";
}

void
SeismicSensor::update()
{
    g_debugDraw.DrawCircle(getPosition(), 1.f, b2Color{0.5f, 0.5f, 0.f, 1.f});

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> distrQuake(8.2f, 9.f);
    static std::uniform_real_distribution<> distrVolcano(.5f, 1.5f);

    float shakeValue = 0.f;

    if (simulation->volcano->isActive()) {
        shakeValue = (float)distrVolcano(gen);
        auto diff = getPosition() - simulation->volcano->getPosition();
        if (diff.Length() < 200.f) {
            shakeValue += 1.f;
        }
    }

    if (simulation->earthquake.isActive()) {
        shakeValue = (float)distrQuake(gen);

        b2Vec2 earthquakePos{simulation->earthquake.epiX, simulation->earthquake.epiY};
        b2Vec2 strength{earthquakePos - getPosition()};
        float attenuation = strength.LengthSquared();
        attenuation /= 1.f;
        attenuation = glm::clamp(attenuation, 0.f, 100000.f);
        shakeValue = glm::mix(shakeValue, 0.001f, attenuation / 100000.f);

    }

    std::string str = std::to_string(shakeValue) + "'Q";
    g_debugDraw.DrawString(getPosition(), str.c_str());

    if (simulation->GetStepCount() % updateFrequency == 0) {
        nlohmann::json j;

        auto pos = getPosition();
        j["pos"] = {{"x", pos.x}, {"y", pos.y}};

        j["shake_val"] = shakeValue;
        j["id"] = object_id;

        Mqtt::getInstance().send("out/sensors", name, j);
    }
}
