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

#include "volcano.h"

#include "framework/draw.h"
#include "simulation.h"
#include "stone.h"

#include <random>

Volcano::Volcano(Simulation *simulation, b2Vec2 pos, float radius)
    : ProximitySensor(simulation, pos, radius, false, true)
{
    this->radius = radius;
}

void
Volcano::update()
{
    MoveToMiddleMouseButtonPressPosition();

    g_debugDraw.DrawSolidCircle(getPosition(), radius, {}, b2Color{1.f, 0.f, 0.f, 1.f});
    g_debugDraw.DrawCircle(getPosition(), radius + 0.2f, b2Color{0.f, 1.0f, 1.f, 1.f});
    g_debugDraw.DrawCircle(getPosition(), radius + 0.4f, b2Color{0.f, 0.0f, 1.f, 1.f});
    g_debugDraw.DrawCircle(getPosition(), radius + 0.8f, b2Color{1.f, 0.0f, 0.f, 1.f});
    g_debugDraw.DrawCircle(getPosition(), radius + 1.2f, b2Color{1.f, 0.0f, 0.f, 1.f});

    if (magnitude == 0.f) {
        return;
    }

    for (auto &&object : objects_inside) {
        b2Vec2 force = object->getPosition() - getPosition();
        object->addForce(b2Vec2{force.x * magnitude, force.y * magnitude});
    }

    if (stepCounter >= continueUntil) {
        magnitude = 0.f;
        world->SetGravity(b2Vec2{0.f, 0.f});
    }

    if (stepCounter % 10 == 0) {

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distrXY(-1.f, 1.f);
        std::uniform_real_distribution<> distrR(1.f, 3.f);

        auto *stone = new Stone{simulation,
                                {getPosition().x + (float)distrXY(gen), getPosition().y + (float)distrXY(gen)},
                                (float)distrR(gen)};
        simulation->SimulateObjectNextFrame(stone);
    }

    stepCounter++;

}

void
Volcano::trigger(float magnitude, int steps)
{
    continueUntil = stepCounter + steps;
    this->magnitude = magnitude;
    simulation->WakeAllObjects();
}

bool
Volcano::isActive() const
{
    if(stepCounter < continueUntil)
    {
        return true;
    }
    return false;
}
