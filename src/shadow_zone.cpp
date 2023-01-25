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

#include "shadow_zone.h"
#include "framework/draw.h"
#include "glm/gtx/rotate_vector.hpp"
#include <glm/glm.hpp>

ShadowZone::ShadowZone(b2Vec2 pos, float rot)
{
    this->pos = pos;
    this->rot = rot;
}

void
ShadowZone::draw()
{
    glm::vec2 v[4];

    glm::vec2 dir = glm::rotate(glm::vec2{1.f, 0.f}, glm::radians(rot));
    glm::vec2 dirN = glm::rotate(dir, glm::radians(90.f));

    v[0] = glm::vec2{pos.x, pos.y} + dir * 1000000.f;
    v[1] = glm::vec2{pos.x, pos.y} + dir * 1000000.f - dirN * 1000000.f;
    v[2] = glm::vec2{pos.x, pos.y} - dir * 1000000.f - dirN * 1000000.f;
    v[3] = glm::vec2{pos.x, pos.y} - dir * 1000000.f;

    g_debugDraw.DrawSolidPolygon(reinterpret_cast<b2Vec2 *>(v), 4, b2Color(0.5f, 0.5f, 0.5f, 0.1f));
}
bool
ShadowZone::inShadowTest(b2Vec2 testPos)
{
    auto rotRad = glm::radians(rot);
    if ((testPos.x - pos.x) * cos(rotRad) - (testPos.y - pos.y) * sin(rotRad) > 0) {
        return true;
    }
    return false;
}
