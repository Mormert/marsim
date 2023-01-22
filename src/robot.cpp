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
#include "laser.h"
#include "mqtt.h"
#include "pickup_sensor.h"
#include "simulation.h"
#include "stone.h"

#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <json.hpp>

Robot::Robot(Simulation* simulation, float width, float length, b2Vec2 position, float angle, float power, float maxSpeed) : Object(simulation)
{
    this->leftAccelerate = 0.f;
    this->rightAccelerate = 0.f;

    this->maxSpeed = maxSpeed;
    this->power = power;

    this->world = world;

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

    pickup_sensor = new PickupSensor{simulation, this, {0.f, 5.f}, 0.5};

    proximity_sensor = new ProximitySensor{simulation, position, 30.f, true};

    laser = new Laser{world, 45.f};
    laser->setPosition(position);

    name = "Robot";
}

void
Robot::attachWheels(std::vector<Wheel *> &wheels)
{
    this->wheels = wheels;
}

void
Robot::update()
{

    // Eliminate sideways velocity
    for (auto &&wheel : wheels) {
        wheel->killSidewaysVelocity();
    }

    if (leftAccelerate != 0.f && getSpeedKMH() < maxSpeed) {
        auto pos = this->wheels[0]->body->GetWorldCenter();
        b2Vec2 force = {0.f, this->power * leftAccelerate};
        this->wheels[0]->body->ApplyForce(wheels[0]->body->GetWorldVector(force), pos, true);
    }

    if (rightAccelerate != 0.f && getSpeedKMH() < maxSpeed) {
        auto pos = this->wheels[1]->body->GetWorldCenter();
        b2Vec2 force = {0.f, this->power * rightAccelerate};
        this->wheels[1]->body->ApplyForce(wheels[1]->body->GetWorldVector(force), pos, true);
    }

    // If going very slowly, stop the car completely.
    if (getSpeedKMH() < 0.2f && leftAccelerate == 0.f && rightAccelerate == 0.f && body->GetAngularVelocity() < 0.1f) {
        completeStopVelocity();
    }

    // Set position for pickup sensor
    auto angle = body->GetAngle();
    auto forward = glm::rotate(glm::vec2{cos(angle), sin(angle)}, glm::radians(90.f));
    forward *= 3.f;
    pickup_sensor->setPosition(getPosition() + b2Vec2{forward.x, forward.y}, angle);
    pickup_sensor->body->SetAwake(true); // Since we manually move the sensor, we need to wake it up all the time.
    pickup_sensor->update();

    // Set position for proximity sensor
    proximity_sensor->setPosition(getPosition(), 0.f);
    proximity_sensor->body->SetAwake(true); // Since we manually move the sensor, we need to wake it up all the time.
    proximity_sensor->update();

    laser->setPosition(getPosition());
    laser->setAngle(glm::degrees(body->GetAngle()) + laserAngleDegrees);
    auto laserHit = laser->castRay();
    if (laserHit && shootNextUpdate) {
        simulation->DestroyObject(laserHit);
    }
    if (shootNextUpdate) {
        shootNextUpdate = false;
    }

    if (updateCounter % 3 == 0) {
        nlohmann::json j;

        auto pos = getPosition();
        j["pos"] = {{"x", pos.x}, {"y", pos.y}, {"r", body->GetAngle()}};
        j["battery"] = 100; // Placeholder
        j["storage"] = storage;

        Mqtt::getInstance().send("simulator", "Robot", j);
    }

    updateCounter++;
}

Robot::~Robot()
{
    for (auto &&wheel : wheels) {
        delete wheel;
    }

    delete pickup_sensor;
    delete proximity_sensor;
    delete laser;
}

void
Robot::pickup()
{
    auto items = getItemsForPickup();
    if (items.empty()) {
        return;
    }

    auto item = items[0];

    storage.push_back(item->name);

    simulation->DestroyObject(item);
}

void
Robot::drop(const std::string &item)
{
    auto it = std::find(storage.begin(), storage.end(), item);

    if (it != storage.end()) {
        auto pos = pickup_sensor->getPosition();
        auto *stone = new Stone{simulation, {pos.x, pos.y}, 1.f};
        simulation->SimulateObject(stone);

        storage.erase(it);
    }
}

std::vector<std::string>
Robot::getStorage()
{
    return storage;
}

std::vector<Object *>
Robot::getItemsForPickup()
{
    return pickup_sensor->getObjectsInside();
}

std::vector<Object *>
Robot::getClosebyObjects()
{
    return proximity_sensor->getObjectsInside();
}

void
Robot::shootLaser()
{
    shootNextUpdate = true;
}
float *
Robot::LaserAngleDegreesPtr()
{
    return &laserAngleDegrees;
}
