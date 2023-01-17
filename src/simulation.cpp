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

#include "simulation.h"
#include "framework/application.h"
#include "robot.h"

Simulation::Simulation() : robot{m_world, 2.f, 3.f, b2Vec2{10.f, 10.f}, 0.f, 480.f, 150.f}
{

    m_world->SetGravity(b2Vec2(0.0f, 0.0f));

    auto wheels =
        std::vector<Wheel>{{m_world, &robot, -1.5f, 0.0f, 0.5f, 0.5f}, {m_world, &robot, 1.5f, 0.0f, 0.5f, 0.5f}};

    robot.attachWheels(wheels);

    b2CircleShape shape;
    shape.m_p.SetZero();
    shape.m_radius = 0.1f;

    float minX = -600.0f;
    float maxX = 600.0f;
    float minY = 400.0f;
    float maxY = 600.0f;

    for (int32 i = 0; i < 4000; ++i) {
        b2BodyDef bd;
        bd.type = b2_staticBody;
        bd.position = b2Vec2(RandomFloat(minX, maxX), RandomFloat(minY, maxY));
        b2Body *body = m_world->CreateBody(&bd);
        body->CreateFixture(&shape, 0.01f);
    }

    {
        b2PolygonShape shape;
        shape.SetAsBox(1.5f, 1.5f);
        b2BodyDef bd;
        bd.type = b2_dynamicBody;
        bd.position.Set(-40.0f, 5.0f);
        bd.bullet = true;
        b2Body *body = m_world->CreateBody(&bd);
        body->CreateFixture(&shape, 1.0f);
        body->SetLinearVelocity(b2Vec2(10.0f, 0.0f));
    }

    {
        b2CircleShape shape;
        shape.m_radius = 2.f;
        b2BodyDef bd;
        bd.type = b2_dynamicBody;
        bd.position.Set(-20.f, -20.f);
        auto *body = m_world->CreateBody(&bd);
        body->CreateFixture(&shape, 1.0f);

        body->SetLinearVelocity(b2Vec2{2.f, 2.f});

        b2PolygonShape shape2;
        shape2.SetAsBox(1.0f, 0.5f, b2Vec2{0.f, -2.25f}, 0.f);
        body->CreateFixture(&shape2, 1.0);

        b2PolygonShape shape3;
        shape3.SetAsBox(1.0f, 0.5f, b2Vec2{0.f, 2.25f}, 0.f);
        body->CreateFixture(&shape3, 1.0);
    }
}

Simulation *
Simulation::Create()
{
    return new Simulation;
}

Simulation::~Simulation() = default;

void
Simulation::Step(Settings &settings)
{

    robot.update();

    Application::Step(settings);
}

void
Simulation::Keyboard(int key)
{

    if (key == GLFW_KEY_W) {
        robot.leftAccelerate = 1.f;
    } else if (key == GLFW_KEY_S) {
        robot.leftAccelerate = -1.f;
    }

    if (key == GLFW_KEY_E) {
        robot.rightAccelerate = 1.f;
    } else if (key == GLFW_KEY_D) {
        robot.rightAccelerate = -1.f;
    }
}
void
Simulation::KeyboardUp(int key)
{
    if (key == GLFW_KEY_W || key == GLFW_KEY_S) {
        robot.leftAccelerate = 0.f;
    }

    if (key == GLFW_KEY_E || key == GLFW_KEY_D) {
        robot.rightAccelerate = 0.f;
    }
}