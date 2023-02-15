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

#ifndef MARSIM_OBJECT_H
#define MARSIM_OBJECT_H

#include "box2d/box2d.h"

#include <string>
#include <vector>

class Simulation;

class Object
{

public:
    Object(Simulation *simulation);
    virtual ~Object() = default;

    virtual b2Vec2 getLocalVelocity();

    float getSpeedKMH();

    b2Vec2 getPosition();

    void completeStopVelocity();

    void addForce(b2Vec2 force);

    void setPosition(b2Vec2 pos, float angle);

    float GetAngularDamping();

    float GetLinearDamping();

    float GetMass();

    virtual std::vector<Object *> getAttachedObjects();

    virtual void update() = 0;

    b2Body *body{};

    bool updateable = true;
    bool terrain_movable = true;

    std::string name{"Unknown Object"};

protected:

    static inline unsigned int id_incrementor{0};
    unsigned int object_id;

    float angularDamping{25.f}, linearDamping{12.5f};

    b2World *world;
    Simulation *simulation;
};

#endif // MARSIM_OBJECT_H
