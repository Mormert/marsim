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

#include "laser.h"
#include "framework/draw.h"
#include "raycast.h"

Laser::Laser(b2World *world, float length)
{
    this->world = world;
    this->length = length;
}

void
Laser::setPosition(b2Vec2 pos)
{
    position = pos;
}

void
Laser::setAngle(float degrees)
{
    this->angle = degrees;
}

void
Laser::setLength(float length)
{
    this->length = length;
}

Object*
Laser::castRay()
{
    float anglerad = b2_pi * this->angle / 180.0f;
    b2Vec2 point1 = position;
    b2Vec2 d(length * cosf(anglerad), length * sinf(anglerad));
    b2Vec2 point2 = point1 + d;

    Raycast callback;
    world->RayCast(&callback, point1, point2);

    if (callback.m_hit) {
        g_debugDraw.DrawPoint(callback.m_point, 5.0f, b2Color(0.4f, 0.9f, 0.4f));
        g_debugDraw.DrawSegment(point1, callback.m_point, b2Color(0.8f, 0.8f, 0.8f));
        b2Vec2 head = callback.m_point + 0.5f * callback.m_normal;
        g_debugDraw.DrawSegment(callback.m_point, head, b2Color(0.9f, 0.9f, 0.4f));
        return callback.m_hit;
    } else {
        g_debugDraw.DrawSegment(point1, point2, b2Color(0.8f, 0.8f, 0.8f));
        return nullptr;
    }
}
