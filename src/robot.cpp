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

#include "robot.h"

Robot::Robot(b2World *world, float width, float length, b2Vec2 position, float angle, float power, float maxSpeed)
{
    this->leftAccelerate = 0.f;
    this->rightAccelerate = 0.f;

    this->maxSpeed = maxSpeed;
    this->power = power;

    b2BodyDef def;
    def.userData.pointer = reinterpret_cast<uintptr_t>(this);
    def.type = b2_dynamicBody;
    def.position = position;
    def.angle = glm::radians(angle);
    def.linearDamping = 0.5;
    def.bullet = true;
    def.angularDamping = 0.8;
    this->body = world->CreateBody(&def);
    body->SetLinearDamping(12.5f);
    body->SetAngularDamping(25.f);

    b2FixtureDef fixdef;
    fixdef.density = 1.0;
    fixdef.friction = 0.5;
    fixdef.restitution = 0.4;
    fixdef.userData.pointer = reinterpret_cast<uintptr_t>(this);
    b2PolygonShape shape;
    shape.SetAsBox(width / 2, length / 2);
    fixdef.shape = &shape;
    body->CreateFixture(&fixdef);
}

void
Robot::attachWheels(std::vector<Wheel> &wheels)
{
    this->wheels = wheels;
}

void
Robot::update()
{

    // Eliminate sideways velocity
    for (auto &&wheel : wheels) {
        wheel.killSidewaysVelocity();
    }

    if (leftAccelerate != 0.f && getSpeedKMH() < maxSpeed) {
        auto pos = this->wheels[0].body->GetWorldCenter();
        b2Vec2 force = {0.f, this->power * leftAccelerate};
        this->wheels[0].body->ApplyForce(wheels[0].body->GetWorldVector(force), pos, true);
    }

    if (rightAccelerate != 0.f && getSpeedKMH() < maxSpeed) {
        auto pos = this->wheels[1].body->GetWorldCenter();
        b2Vec2 force = {0.f, this->power * rightAccelerate};
        this->wheels[1].body->ApplyForce(wheels[1].body->GetWorldVector(force), pos, true);
    }

    // If going very slowly, stop the car completely.
    if (getSpeedKMH() < 0.2f && leftAccelerate == 0.f && rightAccelerate == 0.f && body->GetAngularVelocity() < 0.1f) {
        completeStopVelocity();
    }
}
