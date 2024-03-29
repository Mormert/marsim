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

#include "object.h"
#include "wheel.h"
#include "shadow_zone.h"
#include "Battery.h"


#include <glm/glm.hpp>
#include <json.hpp>
#include <vector>


class PickupSensor;
class ProximitySensor;
class Simulation;
class LidarSensor;
class Laser;
class RobotArm;

class Robot : public Object
{
public:
    std::vector<Wheel*> wheels;
    float leftAccelerate;
    float rightAccelerate;

    float maxSpeed;
    float power;

    Robot(Simulation* simulation, float width, float length, b2Vec2 position, float angle, float power, float max_speed);

    ~Robot() override;

    void attachWheels(std::vector<Wheel*> &wheels);

    void update() override;

    void pickup();

    bool drop(unsigned int index);

    float getStorageMass();

    bool isInShadow();

    void shootLaser();

    void recalculateMass();

    void setLaserAngleDegrees(float deg){ laserAngleDegrees = deg; };

    float* LaserAngleDegreesPtr();

    std::vector<nlohmann::json> getStorage();

    std::vector<Object*> getItemsForPickup();

    std::vector<Object*> getClosebyObjects();

    bool IsBaseLocked();

    void SetBaseLock(bool lock);

    Battery *GetBattery();

    RobotArm* GetArm();

private:
    unsigned int updateCounter{0};

    PickupSensor* pickup_sensor{};
    ProximitySensor* proximity_sensor{};
    RobotArm* robot_arm;

    Laser* laser{};
    float laserAngleDegrees{90.f};
    bool shootNextUpdate{false};

    Simulation* simulation;

    LidarSensor* lidarSensor;

    std::vector<nlohmann::json> storage;
    float storageMass{0.f};


    ShadowZone shadow_zone{b2Vec2{250.f, 0.f}, 45.f};
    Battery *battery;

    friend class Simulation;

};

#endif // MARSIM_ROBOT_H
