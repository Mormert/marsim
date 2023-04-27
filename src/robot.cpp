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
#include "alien.h"
#include "laser.h"
#include "mqtt.h"
#include "pickup_sensor.h"
#include "robot_arm.h"
#include "seismic_sensor.h"
#include "simulation.h"
#include "stone.h"
#include "temperature_sensor.h"
#include "wind_sensor.h"
#include "lidar_sensor.h"

#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <json.hpp>


Robot::Robot(
    Simulation *simulation, float width, float length, b2Vec2 position, float angle, float power, float maxSpeed)
    : Object(simulation)
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
    body->SetLinearDamping(linearDamping);
    body->SetAngularDamping(angularDamping);

    b2FixtureDef fixdef;
    fixdef.filter.groupIndex = -1;
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
    proximity_sensor->shouldTransmitMqtt = false;

    lidarSensor = new LidarSensor{simulation, 30, position};

    laser = new Laser{world, 45.f};
    laser->setPosition(position);

    battery = new Battery(12, 100, 0.01, 1);

    robot_arm = new RobotArm{simulation, body};

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
    //uncomment below if you want to print the battery percentage
    //std::cout << battery->getSoC() * 100 << std::endl;
    battery->reset_current_tick_drain();
    //use of energy just for staying on, for sensors and all
    battery->BatUpdate(1.5);

    robot_arm->update();

    if(!isInShadow()){
        //constant charge with 3 amps per update if not in shadow zone
        battery->BatUpdate(-3);
    }


    // Eliminate sideways velocity
    for (auto &&wheel : wheels) {
        wheel->killSidewaysVelocity();
    }
    //auto frs = (battery->getOCVTable(battery->getSoC())/12.f);
    float frs = battery->getSoC();

    if (leftAccelerate != 0.f && getSpeedKMH() < maxSpeed) {
        auto pos = this->wheels[0]->body->GetWorldCenter();
        b2Vec2 force = {0.f, this->power * frs * leftAccelerate};
        this->wheels[0]->body->ApplyForce(wheels[0]->body->GetWorldVector(force), pos, true);

        battery->BatUpdate((1.f-(getSpeedKMH()/maxSpeed)) * frs * abs(leftAccelerate) * 12);
    }

    if (rightAccelerate != 0.f && getSpeedKMH() < maxSpeed) {
        auto pos = this->wheels[1]->body->GetWorldCenter();
        b2Vec2 force = {0.f, this->power * frs * rightAccelerate};
        this->wheels[1]->body->ApplyForce(wheels[1]->body->GetWorldVector(force), pos, true);
        battery->BatUpdate((1.f-(getSpeedKMH()/maxSpeed)) * frs * abs(rightAccelerate) * 12);
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
        simulation->DestroyObjectNextFrame(laserHit);
        //this battery usage doesn't account for missed shots so fix that

    }
    if (shootNextUpdate) {
        battery->BatUpdate(260);
        shootNextUpdate = false;
    }

    if (updateCounter % 3 == 0) {
        nlohmann::json j;

        auto pos = getPosition();
        j["pos"] = {{"x", pos.x}, {"y", pos.y}, {"r", body->GetAngle()}};
        j["battery"] = (battery->getSoC() * 100); // Yet to be tested
        j["storage"] = storage;
        j["in_shadow"] = isInShadow();

        Mqtt::getInstance().send("out/general", "Robot", j);
    }

    // 60 Hz robot position sync
    if(updateCounter % 1 == 0)
    {
        nlohmann::json j;

        auto pos = getPosition();
        j["pos"] = {{"x", pos.x}, {"y", pos.y}, {"r", body->GetAngle()}};

        //if(body->GetAngularVelocity() > 0.01f || body->GetLinearVelocity().Length() > 0.01f)
        //{
            Mqtt::getInstance().send("out/robotpos", "RobotPos", j);
        //}
    }

    lidarSensor->setPosition(getPosition());
    lidarSensor->update();

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
    delete lidarSensor;
    delete robot_arm;
}

void
Robot::pickup()
{
    auto items = getItemsForPickup();

    for(int i = items.size() - 1; i >= 0; i--)
    {
        if(items[i]->name == "Robot Arm"){
            items.erase(std::begin(items) + i);
        }
    }

    if (items.empty()) {
        Mqtt::getInstance().send("out/pickup", "pickup", "FAIL");
        return;
    }

    auto item = items[0];

    nlohmann::json j;
    j["type"] = item->name;
    j["radius"] = item->body->GetFixtureList()[0].GetShape()->m_radius;
    j["mass"] = item->GetMass();
    j["id"] = item->GetObjectId();

    storage.push_back(j);

    simulation->DestroyObjectNextFrame(item);

    recalculateMass();

    Mqtt::getInstance().send("out/pickup", "pickup", j);
}

bool
Robot::drop(unsigned int index)
{

    if (index >= storage.size()) {
        std::cerr << "Trying to drop item with index outside storage bounds!" << std::endl;
        return false;
    }

    auto item = storage[index];
    std::string itemName = item["type"];
    float itemRadius = item["radius"];
    unsigned int id = item["id"];

    auto pos = pickup_sensor->getPosition();
    if (itemName == "Stone") {
        auto *stone = new Stone{simulation, {pos.x, pos.y}, itemRadius};
        stone->SetObjectId(id);
        simulation->SimulateObjectNextFrame(stone);
    }

    if (itemName == "Alien") {
        auto *alien = new Alien{simulation, simulation->GetTerrain(), pos, body->GetAngle()};
        alien->SetObjectId(id);
        simulation->SimulateObjectNextFrame(alien);
    }

    if(itemName == "Temperature Sensor")
    {
        auto tempSensor = new TemperatureSensor{simulation, {pos.x, pos.y}};
        tempSensor->SetObjectId(id);
        simulation->SimulateObjectNextFrame(tempSensor);
    }

    if(itemName == "Wind Sensor")
    {
        auto windSensor = new WindSensor{simulation, {pos.x, pos.y}};
        windSensor->SetObjectId(id);
        simulation->SimulateObjectNextFrame(windSensor);
    }

    if(itemName == "Seismic Sensor")
    {
        auto seismicSensor = new SeismicSensor{simulation, {pos.x, pos.y}};
        seismicSensor->SetObjectId(id);
        simulation->SimulateObjectNextFrame(seismicSensor);
    }

    storage.erase(storage.begin() + index);

    recalculateMass();

    return true;
}

std::vector<nlohmann::json>
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
bool
Robot::isInShadow()
{
    return simulation->shadow_zone->inShadowTest(getPosition());
}

void
Robot::recalculateMass()
{
    storageMass = 0.f;
    for (auto &&item : storage) {
        storageMass += (float)item["mass"];
    }

    body->ResetMassData();
    auto massData = body->GetMassData();
    b2MassData newMassData{};
    newMassData.I = massData.I + storageMass * 0.5f;
    newMassData.center = massData.center;
    newMassData.mass = massData.mass + storageMass;
    body->SetMassData(&newMassData);
}
float
Robot::getStorageMass()
{
    return storageMass;
}
Battery *
Robot::GetBattery()
{
    return battery;
}
RobotArm *
Robot::GetArm()
{
    return robot_arm;
}
