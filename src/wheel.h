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

#ifndef MARSIM_WHEEL_H
#define MARSIM_WHEEL_H

#include "object.h"

class Robot;

class Wheel : public Object
{

public:
    Wheel(Simulation* simulation, Robot *robot, float x, float y, float width, float height);

    // Get local wheel velocity
    b2Vec2 getLocalVelocity() override;

    // The direction which the wheel points
    b2Vec2 getDirectionVector();

    // Returns a vector which removes sideways velocity and keeps the forward vector velocity
    b2Vec2 getKillVelocityVector();

    // Remove sideways velocity from the wheel
    void killSidewaysVelocity();

    void update() override;

    friend class Robot;

private:
    b2Vec2 position;
    Robot *robot;
};

#endif // MARSIM_WHEEL_H
