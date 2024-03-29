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

#include "wind_sensor.h"
#include "framework/draw.h"
#include "json.hpp"
#include "mqtt.h"
#include "simulation.h"

WindSensor::WindSensor(Simulation *simulation, b2Vec2 pos) : PhysicalWeatherSensor(simulation, pos)
{
    name = "Wind Sensor";
}

void
WindSensor::update()
{
    g_debugDraw.DrawCircle(getPosition(), 1.f, b2Color{1.f, 1.f, 1.f, 1.f});

    auto &&tornadoes = simulation->GetTornados();
    auto &&aliens = simulation->GetAliens();
    //auto &&volcanoes = simulation->GetVolcanoes();

    std::vector<b2Vec2> tornadoStrengths;
    for (auto &&tornado : tornadoes) {
        b2Vec2 strength{tornado.pos - getPosition()};
        float attenuation = 1.f / strength.LengthSquared();
        strength.x *= tornado.magnitude;
        strength.y *= tornado.magnitude;
        strength.x *= attenuation;
        strength.y *= attenuation;
        tornadoStrengths.push_back(strength);
    }

    for (auto &&alien : aliens) {
        b2Vec2 strength{alien.pos - getPosition()};
        float attenuation = 100.f / strength.LengthSquared();
        strength.x *= attenuation;
        strength.y *= attenuation;
        tornadoStrengths.push_back(strength);
    }

    /*for (auto &&volcano : volcanoes) {
        b2Vec2 strength{volcano.pos - getPosition()};
        float attenuation = 1000.f / strength.LengthSquared();
        strength.x *= attenuation;
        strength.y *= attenuation;
        tornadoStrengths.push_back(-strength);
    }*/

    b2Vec2 strength{0.f, 0.f};
    for (auto &&tornadoStrength : tornadoStrengths) {
        strength.x += tornadoStrength.x;
        strength.y += tornadoStrength.y;
    }

    g_debugDraw.DrawSegment(getPosition(), getPosition() + strength, b2Color(0.8f, 0.8f, 0.8f));

    std::string str = "{" + std::to_string(strength.x) + ", " + std::to_string(strength.y) + "}";
    g_debugDraw.DrawString(getPosition(), str.c_str());

    if (simulation->GetStepCount() % updateFrequency == 0) {
        nlohmann::json j;

        auto pos = getPosition();
        j["pos"] = {{"x", pos.x}, {"y", pos.y}};

        j["wind_vec"] = {{"x", strength.x}, {"y", strength.y}};

        j["id"] = object_id;

        Mqtt::getInstance().send("out/sensors", name, j);
    }
}
