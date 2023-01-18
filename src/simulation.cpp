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
#include <iostream>

Simulation::Simulation()
    : robot{m_world, 2.f, 3.f, b2Vec2{10.f, 10.f}, 0.f, 480.f, 150.f}, terrain{"data/lunar_gaussian.png"}
{

    m_world->SetGravity(b2Vec2(0.0f, 0.0f));

    auto wheels =
        std::vector<Wheel>{{m_world, &robot, -1.5f, 0.0f, 0.5f, 0.5f}, {m_world, &robot, 1.5f, 0.0f, 0.5f, 0.5f}};

    robot.attachWheels(wheels);

    b2CircleShape shape;
    shape.m_p.SetZero();
    shape.m_radius = 0.1f;

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
Simulation::ApplySlopeForce()
{
    auto robotPos = robot.getPosition();
    b2Vec2 terrainPixelPos = {robotPos.x + terrain.getTextureWidth() / 2.f,
                              terrain.getTextureHeight() / 2.f - robotPos.y};

    float x = terrainPixelPos.x;
    float y = terrainPixelPos.y;
    auto width = (float)terrain.getTextureWidth();
    auto height = (float)terrain.getTextureHeight();

    float slopeX = terrain.getHeight(x < width - 1.f ? x + 1.f : x, y) - terrain.getHeight(x > 0.f ? x - 1.f : x, y);
    float slopeZ = terrain.getHeight(x, y < height - 1.f ? y + 1.f : y) - terrain.getHeight(x, y > 0.f ? y - 1.f : y);

    if (x == 0 || x == width - 1) {
        slopeX *= 2;
    }

    if (y == 0 || y == height - 1) {
        slopeZ *= 2;
    }

    glm::vec3 slopeDir = {-slopeX * (width - 1), (width - 1), slopeZ * (height - 1)};

    if (glm::abs(slopeDir.x) > 0.1f || glm::abs(slopeDir.z) > 0.1f) {
        slopeDir *= 0.2f; // constant that seems to work
        robot.addForce(b2Vec2{slopeDir.x, slopeDir.z});
    }
}

void
Simulation::Step(Settings &settings)
{

    robot.update();

    g_debugDraw.DrawImageTexture(
        terrain.getTextureID(), {0.f, 0.f}, {(float)terrain.getTextureWidth(), (float)terrain.getTextureHeight()});

    ApplySlopeForce();

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
