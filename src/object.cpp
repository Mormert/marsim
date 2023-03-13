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

#include "object.h"
#include "simulation.h"

b2Vec2
Object::getLocalVelocity()
{
    return body->GetLocalVector(body->GetLinearVelocityFromLocalPoint(b2Vec2(0, 0)));
}

float
Object::getSpeedKMH()
{
    const auto vel = body->GetLinearVelocity();
    return (vel.Length() / 1000.f) * 3600.f;
}

b2Vec2
Object::getPosition()
{
    return body->GetPosition();
}

void
Object::completeStopVelocity()
{
    body->SetLinearVelocity(b2Vec2{0.f, 0.f});
    body->SetAngularVelocity(0.f);
}

void
Object::addForce(b2Vec2 force)
{
    body->ApplyForceToCenter(force, true);
}
void
Object::setPosition(b2Vec2 pos, float angle)
{
    body->SetTransform(pos, angle);
}

Object::Object(Simulation *simulation)
{
    this->simulation = simulation;
    this->world = simulation->GetWorld();
    object_id = id_incrementor++;
}

std::vector<Object *>
Object::getAttachedObjects()
{
    return {};
}
float
Object::GetAngularDamping()
{
    return angularDamping;
}
float
Object::GetLinearDamping()
{
    return linearDamping;
}

float
Object::GetMass()
{
    return body->GetMass();
}
unsigned int
Object::GetObjectId()
{
    return object_id;
}
