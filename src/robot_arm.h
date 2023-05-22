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

#ifndef MARSIM_ROBOT_ARM_H
#define MARSIM_ROBOT_ARM_H

#include "object.h"
#include "box2d/box2d.h"
#include "json.hpp"

class Simulation;

class RobotArm : public Object
{
public:
    explicit RobotArm(Simulation* simulation, b2Body* robotBody);

    void update() override;

    void SetSpeeds(float one, float two, float three);

    void CloseGripper();

    void OpenGripper();

    bool IsGripperOpen();

    bool IsLockFolded();

    void SetLockFolded(bool lock);

    b2Vec2 GetGripperPosition();

    float GetSpeed1();
    float GetSpeed2();
    float GetSpeed3();

    nlohmann::json GetJsonData();

private:
    b2RevoluteJoint* joint1;
    b2RevoluteJoint* joint2;
    b2RevoluteJoint* joint3;

    b2Body* arm3;
    b2Fixture* gripperFixture{};
};

#endif // MARSIM_ROBOT_ARM_H
