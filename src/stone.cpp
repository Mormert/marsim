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

#include "stone.h"

Stone::Stone(b2World *world, b2Vec2 pos, float radius) {
    updateable = false;

    b2BodyDef def;
    def.userData.pointer = reinterpret_cast<uintptr_t>(this);
    def.type = b2_dynamicBody;
    def.position = pos;
    def.angle = 0.f;
    def.linearDamping = 0.5;
    def.bullet = false;
    def.angularDamping = 0.8;
    this->body = world->CreateBody(&def);
    body->SetLinearDamping(12.5f);
    body->SetAngularDamping(25.f);

    b2FixtureDef fixdef;
    fixdef.density = 1.0;
    fixdef.friction = 0.5;
    fixdef.restitution = 0.4;
    fixdef.userData.pointer = reinterpret_cast<uintptr_t>(this);
    b2CircleShape shape;
    shape.m_radius = radius;
    fixdef.shape = &shape;
    body->CreateFixture(&fixdef);

    name = "Stone";
}

void
Stone::update()
{
    // nothing
}
