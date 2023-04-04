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

#include "friction_zone.h"
#include "framework/draw.h"
FrictionZone::FrictionZone(Simulation *simulation, b2Vec2 pos, float radius, float dampingLinear, float dampingAngular)
    : ProximitySensor(simulation, pos, radius, false, true)
{
    this->dampingAngular = dampingAngular;
    this->dampingLinear = dampingLinear;
}

void
FrictionZone::update()
{
    MoveToMiddleMouseButtonPressPosition();

    g_debugDraw.DrawSolidCircle(getPosition(), radius, {}, b2Color{1.f, 0.0f, 1.f, 1.f});
}

void
FrictionZone::OnObjectEnter(Object *o)
{
    if (o->GetLinearDamping() != 0.f) {
        o->body->SetLinearDamping(o->GetLinearDamping() * 100.f);
    }
    if (o->GetAngularDamping() != 0.f) {
        o->body->SetAngularDamping(o->GetAngularDamping() * 100.f);
    }
}

void
FrictionZone::OnObjectLeave(Object *o)
{
    if (o->GetLinearDamping() != 0.f) {
        o->body->SetLinearDamping(o->GetLinearDamping());
    }
    if (o->GetAngularDamping() != 0.f) {
        o->body->SetAngularDamping(o->GetAngularDamping());
    }
}
