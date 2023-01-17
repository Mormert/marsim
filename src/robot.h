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

#ifndef MARSIM_ROBOT_H
#define MARSIM_ROBOT_H

#include "wheel.h"

#include <glm/glm.hpp>
#include <vector>

class Robot
{
public:
    std::vector<Wheel> wheels;
    float leftAccelerate;
    float rightAccelerate;

    float maxSpeed;
    float power;

    Robot(b2World *world, float width, float length, b2Vec2 position, float angle, float power, float max_speed);

    void attachWheels(std::vector<Wheel> &wheels);

    b2Vec2 getLocalVelocity();

    float getSpeedKMH();

    /* // Setting the speed should only be used in special cases, such as when the robot is stopped!
     void setSpeedKMH(float speed) {


         return;

         const auto velocity = body->GetLinearVelocity();
         const auto normalizedVelocityGLM = glm::normalize(glm::vec2{velocity.x, velocity.y});

         body->SetLinearVelocity(b2Vec2{normalizedVelocityGLM.x * ((speed * 1000.f) / 3600.f),
                                        normalizedVelocityGLM.y * ((speed * 1000.f) / 3600.f)});
     }*/

    void completeStopVelocity();

    void update();

    b2Body *body;
};

#endif // MARSIM_ROBOT_H