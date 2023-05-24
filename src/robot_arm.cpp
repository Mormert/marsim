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

#include "robot_arm.h"
#include "glm/trigonometric.hpp"
#include "mqtt.h"
#include "simulation.h"

RobotArm::RobotArm(Simulation *simulation, b2Body *robotBody) : Object(simulation)
{
    auto world = simulation->GetWorld();

    b2BodyDef def;

    def.type = b2_dynamicBody;
    def.userData.pointer = reinterpret_cast<uintptr_t>(this);
    def.position = robotBody->GetPosition();
    def.linearDamping = 0.25;
    def.angularDamping = 0.2;
    auto body1 = world->CreateBody(&def);
    auto body2 = world->CreateBody(&def);
    auto body3 = world->CreateBody(&def);
    body = body1;
    arm3 = body3;

    b2FixtureDef fixdef;

    fixdef.filter.groupIndex = -1;
    fixdef.userData.pointer = reinterpret_cast<uintptr_t>(this);
    fixdef.density = 0.5;
    fixdef.friction = 0.1;
    fixdef.restitution = 0.1;
    b2PolygonShape shape;
    shape.SetAsBox(0.2, 2);
    fixdef.shape = &shape;
    body1->CreateFixture(&fixdef);
    body2->CreateFixture(&fixdef);
    body3->CreateFixture(&fixdef);

    b2RevoluteJointDef jd1;
    jd1.bodyA = robotBody;
    jd1.localAnchorB.Set(0.f, -1.5f);
    jd1.enableMotor = true;
    jd1.maxMotorTorque = 1000.0f;

    constexpr float pi = 3.14159265358979;

    jd1.motorSpeed = 0.0f;
    jd1.localAnchorA.SetZero();
    jd1.bodyB = body1;
    jd1.enableLimit = false;
    jd1.lowerAngle = pi - 0.02f;
    jd1.upperAngle = pi + 0.02f;
    joint1 = (b2RevoluteJoint *)world->CreateJoint(&jd1);

    b2RevoluteJointDef jd2;
    jd2.bodyA = body1;
    jd2.localAnchorB.Set(0.f, -1.5f);
    jd2.enableMotor = true;
    jd2.maxMotorTorque = 1000.0f;


    jd2.motorSpeed = 0.0f;
    jd2.localAnchorA.Set(0.f, 1.5f);
    jd2.bodyB = body2;
    jd2.enableLimit = false;
    jd2.lowerAngle = -0.02f;
    jd2.upperAngle = 0.02f;
    joint2 = (b2RevoluteJoint *)world->CreateJoint(&jd2);

    b2RevoluteJointDef jd3;
    jd3.bodyA = body2;
    jd3.localAnchorB.Set(0.f, -1.5f);
    jd3.enableMotor = true;
    jd3.maxMotorTorque = 1000.0f;

    jd3.motorSpeed = 0.0f;
    jd3.localAnchorA.Set(0.f, 1.5f);
    jd3.bodyB = body3;
    jd3.enableLimit = false;
    jd3.lowerAngle = pi - 0.02f;
    jd3.upperAngle = pi + 0.02f;
    joint3 = (b2RevoluteJoint *)world->CreateJoint(&jd3);

    b2BodyDef defGripper;

    defGripper.type = b2_dynamicBody;
    defGripper.userData.pointer = reinterpret_cast<uintptr_t>(this);
    defGripper.position = body3->GetPosition();
    defGripper.linearDamping = 0.25;
    defGripper.angularDamping = 0.2;
    auto bodyGripper1 = world->CreateBody(&defGripper);
    auto bodyGripper2 = world->CreateBody(&defGripper);

    b2FixtureDef fixdefGripper;

    fixdefGripper.filter.groupIndex = -1;
    fixdefGripper.userData.pointer = reinterpret_cast<uintptr_t>(this);
    fixdefGripper.density = 0.5;
    fixdefGripper.friction = 0.1;
    fixdefGripper.restitution = 0.1;
    b2PolygonShape shapeGripper;
    shapeGripper.SetAsBox(0.1, 0.8);
    fixdefGripper.shape = &shapeGripper;
    bodyGripper1->CreateFixture(&fixdefGripper);
    bodyGripper2->CreateFixture(&fixdefGripper);

    b2WeldJointDef jdGripper;
    jdGripper.bodyA = bodyGripper1;
    jdGripper.bodyB = body3;
    jdGripper.localAnchorB.Set(0.7f, 2.5f);
    jdGripper.referenceAngle = glm::radians(45.f);
    world->CreateJoint(&jdGripper);

    jdGripper.bodyA = bodyGripper2;
    jdGripper.referenceAngle = glm::radians(-45.f);
    jdGripper.localAnchorB.Set(-0.7f, 2.5f);
    world->CreateJoint(&jdGripper);

    updateable = true;
    terrain_movable = false;

    name = "Robot Arm";
}
void
RobotArm::SetSpeeds(float one, float two, float three)
{
    joint1->SetMotorSpeed(one);
    joint2->SetMotorSpeed(two);
    joint3->SetMotorSpeed(three);
}
float
RobotArm::GetSpeed1()
{
    return joint1->GetMotorSpeed();
}
float
RobotArm::GetSpeed2()
{
    return joint2->GetMotorSpeed();
}
float
RobotArm::GetSpeed3()
{
    return joint3->GetMotorSpeed();
}

void
RobotArm::CloseGripper()
{
    if (gripperFixture) {
        return;
    }

    b2FixtureDef fixdefGripper;

    fixdefGripper.filter.groupIndex = -1;
    fixdefGripper.userData.pointer = reinterpret_cast<uintptr_t>(this);
    fixdefGripper.density = 0.5;
    fixdefGripper.friction = 0.1;
    fixdefGripper.restitution = 0.1;
    b2PolygonShape shapeGripper;
    shapeGripper.SetAsBox(0.1, 1.4, b2Vec2{0.f, 4.5f}, glm::radians(90.f));
    fixdefGripper.shape = &shapeGripper;

    gripperFixture =  arm3->CreateFixture(&fixdefGripper);
}

void
RobotArm::OpenGripper()
{
    if (!gripperFixture) {
        return;
    }

    arm3->DestroyFixture(gripperFixture);

    gripperFixture = nullptr;
}

bool
RobotArm::IsGripperOpen()
{
    return !gripperFixture;
}
nlohmann::json
RobotArm::GetJsonData()
{
    nlohmann::json j;

    j["arm1_motorSpeed"] = joint1->GetMotorSpeed();
    j["arm1_jointAngle"] = joint1->GetJointAngle();
    j["arm1_jointSpeed"] = joint1->GetJointSpeed();
    j["arm2_motorSpeed"] = joint2->GetMotorSpeed();
    j["arm2_jointAngle"] = joint2->GetJointAngle();
    j["arm2_jointSpeed"] = joint2->GetJointSpeed();
    j["arm3_motorSpeed"] = joint3->GetMotorSpeed();
    j["arm3_jointAngle"] = joint3->GetJointAngle();
    j["arm3_jointSpeed"] = joint3->GetJointSpeed();
    j["arm_opened"] = IsGripperOpen();
    j["arm_fold_locked"] = IsLockFolded();

    return j;
}

void
RobotArm::update()
{
    if(simulation->GetStepCount() % 3 == 0)
    {
        Mqtt::getInstance().send("out/arm", "arm", GetJsonData());
    }
}

bool
RobotArm::IsLockFolded()
{
    return joint1->IsLimitEnabled();
}

void
RobotArm::SetLockFolded(bool lock)
{
    joint1->EnableLimit(lock);
    joint2->EnableLimit(lock);
    joint3->EnableLimit(lock);
}
b2Vec2
RobotArm::GetGripperPosition()
{
    constexpr float pi = 3.14159265359879;
    return arm3->GetPosition() + b2Vec2{cos(arm3->GetAngle() + pi/2.f)*3.4f, sin(arm3->GetAngle() + pi/2.f)*3.4f};
}
