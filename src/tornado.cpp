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

#include "tornado.h"
#include "framework/draw.h"
#include "simulation.h"

Tornado::Tornado(Simulation *simulation, b2Vec2 pos, float radius, float magnitude)
    : ProximitySensor(simulation, pos, radius, true, true)
{
    this->radius = radius;
    this->magnitude = magnitude;

    FindNewMoveToTarget();
}
void
Tornado::update()
{
    TornadoMovement();

    MoveToMiddleMouseButtonPressPosition();

    g_debugDraw.DrawSolidCircle(getPosition(), radius, {}, b2Color{1.f, 0.0f, 0.f, 1.f});
    g_debugDraw.DrawCircle(getPosition(), radius + 0.2f, b2Color{1.f, 0.0f, 0.f, 1.f});
    g_debugDraw.DrawCircle(getPosition(), radius + 0.4f, b2Color{1.f, 1.0f, 0.f, 1.f});

    for (auto &&object : objects_inside) {
        b2Vec2 force = getPosition() - object->getPosition();
        object->addForce(b2Vec2{force.x * magnitude, force.y * magnitude});
    }
}

void
Tornado::TornadoMovement()
{

    b2Vec2 d = target - getPosition();
    b2Vec2 dir = d;
    dir.Normalize();
    dir.x *= 0.0f;
    dir.y *= 0.0f;

    setPosition(getPosition() + dir, 0.f);

    body->SetAwake(true);

    if (d.LengthSquared() < 2.f) {
        FindNewMoveToTarget();
    }
}

void
Tornado::FindNewMoveToTarget()
{
    int rangeY =
        simulation->GetTerrain()->getTextureHeight() / 2 - (-simulation->GetTerrain()->getTextureHeight() / 2) + 1;
    int y = std::rand() % rangeY + (-simulation->GetTerrain()->getTextureHeight() / 2);

    int rangeX =
        simulation->GetTerrain()->getTextureWidth() / 2 - (-simulation->GetTerrain()->getTextureWidth() / 2) + 1;
    int x = std::rand() % rangeX + (-simulation->GetTerrain()->getTextureWidth() / 2);

    target = b2Vec2{(float)x, (float)y};
}
