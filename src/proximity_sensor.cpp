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

#include "proximity_sensor.h"
ProximitySensor::ProximitySensor(b2World *world, b2Vec2 pos, float radius)
{
    moveable = false;

    b2BodyDef def;
    def.userData.pointer = reinterpret_cast<uintptr_t>(this);
    def.type = b2_staticBody;
    def.position = pos;
    def.bullet = false;
    this->body = world->CreateBody(&def);

    b2CircleShape shape;
    shape.m_radius = radius;

    b2FixtureDef fd;
    fd.shape = &shape;
    fd.isSensor = true;
    fd.userData.pointer = reinterpret_cast<uintptr_t>(this);
    body->CreateFixture(&fd);
}

void
ProximitySensor::update()
{
}

void
ProximitySensor::ObjectEnter(Object *other)
{
    objects_inside.push_back(other);
}

void
ProximitySensor::ObjectLeave(Object *other)
{
    auto it = std::find(objects_inside.begin(), objects_inside.end(), other);
    if (it != objects_inside.end()) {
        objects_inside.erase(it);
    }
}
