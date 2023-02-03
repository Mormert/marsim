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

#include "pickup_sensor.h"

#include "robot.h"

PickupSensor::PickupSensor(Simulation* simulation, Robot *robot, b2Vec2 pos, float radius) : ProximitySensor(simulation)
{
    terrain_movable = false;
    updateFrequency = 10;

    this->radius = radius;

    b2BodyDef def;
    def.userData.pointer = reinterpret_cast<uintptr_t>(this);
    def.type = b2_kinematicBody;
    def.position = robot->body->GetWorldPoint(pos);
    def.angle = robot->body->GetAngle();
    body = world->CreateBody(&def);

    // Triangle
    b2Vec2 vertices[3];
    vertices[0].Set(0, -1);
    vertices[1].Set(-1.5,1);
    vertices[2].Set(1.5,1);

    b2PolygonShape shape;
    shape.Set(vertices, 3);

    b2FixtureDef fixdef;
    fixdef.isSensor = true;
    fixdef.shape = &shape;
    fixdef.userData.pointer = reinterpret_cast<uintptr_t>(this);
    body->CreateFixture(&fixdef);

    name = "Pickup Sensor";
}
