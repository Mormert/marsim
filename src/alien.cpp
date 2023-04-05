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

#include "alien.h"
#include "framework/draw.h"
#include "proximity_sensor.h"
#include "robot.h"
#include "simulation.h"

Alien::Alien(Simulation *simulation, Terrain *terrain, b2Vec2 pos, float rotation) : Object(simulation)
{
    this->terrain = terrain;

    updateable = true;
    terrain_movable = true;

    b2BodyDef def;
    def.userData.pointer = reinterpret_cast<uintptr_t>(this);
    def.type = b2_dynamicBody;
    def.position = pos;
    def.angle = rotation;
    def.linearDamping = 0.5;
    def.bullet = false;
    def.angularDamping = 0.8;
    this->body = world->CreateBody(&def);
    body->SetLinearDamping(linearDamping);
    body->SetAngularDamping(angularDamping);

    b2FixtureDef fixdef;
    fixdef.density = 1.0;
    fixdef.friction = 0.5;
    fixdef.restitution = 0.4;
    fixdef.userData.pointer = reinterpret_cast<uintptr_t>(this);
    b2PolygonShape shape;
    shape.SetAsBox(1.f, 0.5f);
    fixdef.shape = &shape;
    body->CreateFixture(&fixdef);

    sights_sensor = new ProximitySensor{simulation, pos, sight_distance, true, false};
    simulation->SimulateObjectNextFrame(sights_sensor);

    name = "Alien";

    FindNewMoveToTarget();
}

void
Alien::update()
{
    sights_sensor->setPosition(getPosition(), 0.f);
    sights_sensor->body->SetAwake(true); // Since we manually move the sensor, we need to wake it up all the time.

    g_debugDraw.DrawSolidCircle(getPosition(), sight_distance, {}, b2Color{1.f, 0.2f, 0.2f, 1.f});

    if (state == AlienState::WALK_AROUND) {
        WalkAround();
        for (auto &&object : sights_sensor->getObjectsInside()) {
            if (auto robot = dynamic_cast<Robot *>(object)) {
                state = AlienState::CHASE;
                sight_distance = 30.f;
                sights_sensor->setRadius(sight_distance);
                break;
            }
        }
    } else if (state == AlienState::CHASE) {
        bool foundRobot = false;
        for (auto &&object : sights_sensor->getObjectsInside()) {
            if (auto robot = dynamic_cast<Robot *>(object)) {
                auto dir = robot->getPosition() - getPosition();
                dir.Normalize();
                dir.x *= 125.f;
                dir.y *= 125.f;
                addForce(dir);

                foundRobot = true;
                break;
            }
        }
        if (!foundRobot) {
            state = AlienState::WALK_AROUND;
            sight_distance = 15.f;
            sights_sensor->setRadius(sight_distance);
        }
    }
}

std::vector<Object *>
Alien::getAttachedObjects()
{
    return {sights_sensor};
}

void
Alien::WalkAround()
{
    b2Vec2 dir = getPosition() - target;
    dir.Normalize();
    dir.x *= 100.f;
    dir.y *= 100.f;

    addForce(dir);

    if (simulation->GetStepCount() % 1200 == 0) {
        FindNewMoveToTarget();
    }
}

void
Alien::FindNewMoveToTarget()
{
    int rangeY =
        simulation->GetTerrain()->getTextureHeight() / 2 - (-simulation->GetTerrain()->getTextureHeight() / 2) + 1;
    int y = std::rand() % rangeY + (-simulation->GetTerrain()->getTextureHeight() / 2);

    int rangeX =
        simulation->GetTerrain()->getTextureWidth() / 2 - (-simulation->GetTerrain()->getTextureWidth() / 2) + 1;
    int x = std::rand() % rangeX + (-simulation->GetTerrain()->getTextureWidth() / 2);

    target = b2Vec2{(float)x, (float)y};
}
