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
#include "proximity_sensor.h"
#include "robot.h"
#include "stone.h"
#include <iostream>
#include <vector>

Simulation::Simulation() : terrain{"data/lunar_gaussian.png"}
{

    m_world->SetGravity(b2Vec2(0.0f, 0.0f));

    robot = new Robot{m_world, 2.f, 3.f, b2Vec2{10.f, 10.f}, 0.f, 480.f, 150.f};
    auto wheels = std::vector<Wheel *>{new Wheel{m_world, robot, -1.5f, 0.0f, 0.5f, 0.5f},
                                       new Wheel{m_world, robot, 1.5f, 0.0f, 0.5f, 0.5f}};

    robot->attachWheels(wheels);
    SimulateObject(robot);

    auto *stone = new Stone{m_world, {15, 15.f}, 1.f};
    SimulateObject(stone);

    auto proximity_sensor = new ProximitySensor{m_world, {40.f, 40.f}, 3.f};
    SimulateObject(proximity_sensor);
}

Simulation *
Simulation::Create()
{
    return new Simulation;
}

Simulation::~Simulation()
{
    for (auto &&object : objects) {
        delete object;
    }

    objects = {};
};

void
Simulation::UpdateObjects()
{
    for (auto &&object : objects) {
        if (object->updateable)
            object->update();
    }
}

void
Simulation::ApplySlopeForce()
{
    for (auto &&object : objects) {
        if (!object->terrain_movable) {
            continue;
        }

        auto robotPos = object->getPosition();
        b2Vec2 terrainPixelPos = {robotPos.x + terrain.getTextureWidth() / 2.f,
                                  terrain.getTextureHeight() / 2.f - robotPos.y};

        float x = terrainPixelPos.x;
        float y = terrainPixelPos.y;
        auto width = (float)terrain.getTextureWidth();
        auto height = (float)terrain.getTextureHeight();

        float slopeX =
            terrain.getHeight(x < width - 1.f ? x + 1.f : x, y) - terrain.getHeight(x > 0.f ? x - 1.f : x, y);
        float slopeZ =
            terrain.getHeight(x, y < height - 1.f ? y + 1.f : y) - terrain.getHeight(x, y > 0.f ? y - 1.f : y);

        if (x == 0 || x == width - 1) {
            slopeX *= 2;
        }

        if (y == 0 || y == height - 1) {
            slopeZ *= 2;
        }

        glm::vec3 slopeDir = {-slopeX * (width - 1), (width - 1), slopeZ * (height - 1)};

        if (glm::abs(slopeDir.x) > 0.1f || glm::abs(slopeDir.z) > 0.1f) {
            slopeDir *= 0.2f; // constant that seems to work
            object->addForce(b2Vec2{slopeDir.x, slopeDir.z});
        }
    }
}

void
Simulation::Step(Settings &settings)
{
    UpdateObjects();

    ApplySlopeForce();

    g_debugDraw.DrawImageTexture(
        terrain.getTextureID(), {0.f, 0.f}, {(float)terrain.getTextureWidth(), (float)terrain.getTextureHeight()});

    Application::Step(settings);
}

void
Simulation::Keyboard(int key)
{

    if (key == GLFW_KEY_W) {
        robot->leftAccelerate = 1.f;
    } else if (key == GLFW_KEY_S) {
        robot->leftAccelerate = -1.f;
    }

    if (key == GLFW_KEY_E) {
        robot->rightAccelerate = 1.f;
    } else if (key == GLFW_KEY_D) {
        robot->rightAccelerate = -1.f;
    }
}
void
Simulation::KeyboardUp(int key)
{
    if (key == GLFW_KEY_W || key == GLFW_KEY_S) {
        robot->leftAccelerate = 0.f;
    }

    if (key == GLFW_KEY_E || key == GLFW_KEY_D) {
        robot->rightAccelerate = 0.f;
    }
}
void

Simulation::SimulateObject(Object *object)
{
    objects.push_back(object);
}

void
Simulation::DestroyObject(Object *object)
{
    auto it = std::find(objects.begin(), objects.end(), object);
    if (it != objects.end()) {
        objects.erase(it);
    }

    delete object;
}

void
Simulation::BeginContact(b2Contact *contact)
{

    b2Fixture *fixtureA = contact->GetFixtureA();
    b2Fixture *fixtureB = contact->GetFixtureB();

    auto *a = reinterpret_cast<Object *>(fixtureA->GetUserData().pointer);
    auto *b = reinterpret_cast<Object *>(fixtureB->GetUserData().pointer);

    if (auto sensor = dynamic_cast<ProximitySensor *>(a)) {
        std::cout << "COLLIDE WITH SENSOR A " << b->name << std::endl;
        sensor->ObjectEnter(b);
    }

    if (auto sensor = dynamic_cast<ProximitySensor *>(b)) {
        std::cout << "COLLIDE WITH SENSOR B " << a->name << std::endl;
        sensor->ObjectEnter(a);
    }
}

void
Simulation::EndContact(b2Contact *contact)
{
    b2Fixture *fixtureA = contact->GetFixtureA();
    b2Fixture *fixtureB = contact->GetFixtureB();

    auto *a = reinterpret_cast<Object *>(fixtureA->GetUserData().pointer);
    auto *b = reinterpret_cast<Object *>(fixtureB->GetUserData().pointer);

    if (auto sensor = dynamic_cast<ProximitySensor *>(a)) {
        std::cout << "LEAVE SENSOR A " << b->name << std::endl;
        sensor->ObjectLeave(b);
    }

    if (auto sensor = dynamic_cast<ProximitySensor *>(b)) {
        std::cout << "LEAVE SENSOR B " << a->name << std::endl;
        sensor->ObjectLeave(a);
    }
}
