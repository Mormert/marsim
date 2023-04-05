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

#ifndef MARSIM_ALIEN_H
#define MARSIM_ALIEN_H

#include "object.h"
#include "terrain.h"
#include <box2d/box2d.h>

class ProximitySensor;

class Alien : public Object
{
    enum class AlienState { WALK_AROUND, CHASE };

public:
    Alien(Simulation *simulation, Terrain *terrain, b2Vec2 pos, float rotation);

    std::vector<Object *> getAttachedObjects() override;

    void update() override;

private:
    Terrain *terrain;

    void FindNewMoveToTarget();
    void WalkAround();
    b2Vec2 target{};

    ProximitySensor *sights_sensor;
    float sight_distance{15.f};
    AlienState state{AlienState::WALK_AROUND};
};

#endif // MARSIM_ALIEN_H
