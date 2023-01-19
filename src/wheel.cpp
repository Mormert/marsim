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

#include "wheel.h"
#include "robot.h"

#include <glm/gtx/rotate_vector.hpp>

Wheel::Wheel(b2World *world, Robot *robot, float x, float y, float width, float height)
{
    position = {x, y};
    this->robot = robot;

    { // Wheel body
        b2BodyDef def;
        def.type = b2_dynamicBody;
        def.position = robot->body->GetWorldPoint(position);
        def.angle = robot->body->GetAngle();
        def.userData.pointer = reinterpret_cast<uintptr_t>(this);
        body = world->CreateBody(&def);
    }

    { // Wheel shape
        b2FixtureDef fixdef;
        fixdef.userData.pointer = reinterpret_cast<uintptr_t>(this);
        fixdef.density = 100.f;
        fixdef.isSensor = true;
        b2PolygonShape shape;
        shape.SetAsBox(width / 2.f, height / 2.f);
        fixdef.shape = &shape;
        body->CreateFixture(&fixdef);
    }

    { // Connect wheel body to robot body with joints
        b2PrismaticJointDef jointDef;
        jointDef.Initialize(robot->body, body, body->GetWorldCenter(), b2Vec2{1.f, 0.f});
        jointDef.enableLimit = true;
        jointDef.upperTranslation = 0;
        jointDef.lowerTranslation = 0;

        world->CreateJoint(&jointDef);
    }

    name = "Robot Wheel";
    updateable = false;

    // Movable false because robot is movable instead
    terrain_movable = false;
}

b2Vec2
Wheel::getLocalVelocity()
{
    return robot->body->GetLocalVector(robot->body->GetLinearVelocityFromLocalPoint(position));
}

b2Vec2
Wheel::getDirectionVector()
{
    const auto localVelocity = getLocalVelocity();
    if (localVelocity.y > 0.f) {
        const auto v = glm::rotate(glm::vec2{0.f, 1.f}, body->GetAngle());
        return {v.x, v.y};
    } else {
        const auto v = glm::rotate(glm::vec2{0.f, -1.f}, body->GetAngle());
        return {v.x, v.y};
    }
}

b2Vec2
Wheel::getKillVelocityVector()
{
    const auto velocity = body->GetLinearVelocity();
    const auto sidewaysAxis = getDirectionVector();
    const auto dot = glm::dot(glm::vec2{velocity.x, velocity.y}, glm::vec2{sidewaysAxis.x, sidewaysAxis.y});
    return b2Vec2{sidewaysAxis.x * dot, sidewaysAxis.y * dot};
}

void
Wheel::killSidewaysVelocity()
{
    body->SetLinearVelocity(getKillVelocityVector());
}

void
Wheel::update()
{
}
