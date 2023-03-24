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

#include "simulation.h"
#include "alien.h"
#include "framework/application.h"
#include "friction_zone.h"
#include "mqtt.h"
#include "proximity_sensor.h"
#include "robot.h"
#include "seismic_sensor.h"
#include "stone.h"
#include "temperature_sensor.h"
#include "tornado.h"
#include "volcano.h"
#include "wind_sensor.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

Simulation::Simulation(const SimulationSetup &setup) : earthquake{m_world, this}
{

    this->setup = setup;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<> imageScaleFactorMultiplierDistr(setup.satelliteImageScaleFactorMultiplierMin,
                                                                     setup.satelliteImageScaleFactorMultiplierMax);

    imageScaleFactorMultiplier = setup.satelliteImageScaleFactor * (float)imageScaleFactorMultiplierDistr(gen);

    Terrain::terrainScaling = imageScaleFactorMultiplier;
    Mqtt::requestImagePath = setup.satelliteImagePath;

    GenerateBlurredTerrain();

    m_world->SetGravity(b2Vec2(0.0f, 0.0f));

    shadow_zone = new ShadowZone({setup.shadowFrontierX, setup.shadowFrontierY}, setup.shadowFrontierR);

    robot = new Robot{this, 2.f, 3.f, b2Vec2{setup.robotX, setup.robotY}, setup.robotR, 480.f, 150.f};
    auto wheels = std::vector<Wheel *>{new Wheel{this, robot, -1.5f, 0.0f, 0.5f, 0.5f},
                                       new Wheel{this, robot, 1.5f, 0.0f, 0.5f, 0.5f}};

    robot->simulation = this;
    robot->attachWheels(wheels);
    SimulateObject(robot);

    std::uniform_real_distribution<> distrX(setup.objectGenerationMinX * imageScaleFactorMultiplier,
                                            setup.objectGenerationMaxX * imageScaleFactorMultiplier);
    std::uniform_real_distribution<> distrY(setup.objectGenerationMinY * imageScaleFactorMultiplier,
                                            setup.objectGenerationMaxY * imageScaleFactorMultiplier);
    std::uniform_real_distribution<> distrR(1.f, 3.f);
    for (int i = 0; i < setup.stonesAmount; i++) {
        auto *stone = new Stone{this, {(float)distrX(gen), (float)distrY(gen)}, (float)distrR(gen)};
        SimulateObject(stone);
    }

    for (int i = 0; i < setup.aliensAmount; i++) {
        auto alien = new Alien{this, terrain, {(float)distrX(gen), (float)distrY(gen)}, (float)distrY(gen)};
        SimulateObject(alien);
    }

    std::uniform_int_distribution<> distSensorRadius(12, 24);

    /* // Removed proximity sensors, replaced with "global sensor-sensor-thingey"
    for (int i = 0; i < setup.proximitySensorsAmount; i++) {
        auto proximity_sensor =
            new ProximitySensor{this, {(float)distrX(gen), (float)distrY(gen)}, 500.f};
        SimulateObject(proximity_sensor);
    }
    */

    for (int i = 0; i < setup.frictionZonesAmount; i++) {
        auto frictionZone =
            new FrictionZone{this, {(float)distrX(gen), (float)distrY(gen)}, (float)distSensorRadius(gen), 22.5f, 40.f};
        SimulateObject(frictionZone);
    }

    for (int i = 0; i < setup.tornadoesAmount; i++) {
        auto tornado =
            new Tornado{this, {(float)distrX(gen), (float)distrY(gen)}, (float)distSensorRadius(gen), 1000.f};
        SimulateObject(tornado);
    }

    for (int i = 0; i < setup.windSensorsAmount; i++) {
        auto windSensor = new WindSensor{this, {(float)distrX(gen), (float)distrY(gen)}};
        SimulateObject(windSensor);
    }

    for (int i = 0; i < setup.seismicSensorsAmount; i++) {
        auto seismicSensor = new SeismicSensor{this, {(float)distrX(gen), (float)distrY(gen)}};
        SimulateObject(seismicSensor);
    }

    for (int i = 0; i < setup.tempSensorsAmount; i++) {
        auto tempSensor = new TemperatureSensor{this, {(float)distrX(gen), (float)distrY(gen)}};
        SimulateObject(tempSensor);
    }

    volcano = new Volcano{this, {(float)distrX(gen), (float)distrY(gen)}, (float)distSensorRadius(gen) * 3.f};
    SimulateObject(volcano);

    TopicSetting ts;
    ts.waitForMQTTConnection = true;
    ts.retained = true;
    ts.maxMessages = 1;
    Mqtt::getInstance().overrideTopicSettings("sim/out/restart", ts);

    nlohmann::json j = GetGeneralInfo();

    Mqtt::getInstance().send("sim/out/restart", "restart", j);
}

Simulation *
Simulation::Create(const std::string &initJson)
{
    SimulationSetup setup;

    std::ifstream file{initJson};

    if (file.good()) {
        nlohmann::json j = nlohmann::json::parse(file);

        try {
            setup.robotX = j["robotX"];
            setup.robotY = j["robotY"];
            setup.robotR = j["robotR"];
            setup.stonesAmount = j["stonesAmount"];
            setup.aliensAmount = j["aliensAmount"];
           // setup.proximitySensorsAmount = j["proximitySensorsAmount"];
            setup.frictionZonesAmount = j["frictionZonesAmount"];
            setup.tornadoesAmount = j["tornadoesAmount"];
            setup.windSensorsAmount = j["windSensorsAmount"];
            setup.seismicSensorsAmount = j["seismicSensorsAmount"];
            setup.tempSensorsAmount = j["tempSensorsAmount"];
            setup.shadowFrontierX = j["shadowFrontierX"];
            setup.shadowFrontierY = j["shadowFrontierY"];
            setup.shadowFrontierR = j["shadowFrontierR"];
            setup.satelliteImageScaleFactor = j["satelliteImageScale"];
            setup.satelliteImageScaleFactorMultiplierMin = j["satelliteImageScaleFactorMultiplierMin"];
            setup.satelliteImageScaleFactorMultiplierMax = j["satelliteImageScaleFactorMultiplierMax"];
            setup.satelliteImagePath = j["satelliteImagePath"];
            setup.objectGenerationMinX = j["objectGenerationMinX"];
            setup.objectGenerationMaxX = j["objectGenerationMaxX"];
            setup.objectGenerationMinY = j["objectGenerationMinY"];
            setup.objectGenerationMaxY = j["objectGenerationMaxY"];
        } catch (std::exception &e) {
            std::cerr << "Failed to parse init.json file: " << e.what() << "\nUsing default simulator settings!"
                      << std::endl;
        }

        std::cout << "Parsed custom init json file successfully!" << std::endl;
    } else {
        std::cerr << "Failed to open and read " << initJson << "! Using default settings!" << std::endl;
    }

    return new Simulation(setup);
}

Simulation::~Simulation()
{
    for (auto &&object : objects) {
        delete object;
    }

    objects = {};

    delete terrain;
    delete shadow_zone;
};

void
Simulation::UpdateObjects()
{

    volcanoDatas.clear();
    tornadoDatas.clear();

    for (auto &&object : objects) {
        if (object->updateable) {
            object->update();
        }
        if (auto tornado = dynamic_cast<Tornado *>(object)) {
            tornadoDatas.push_back({tornado->getPosition(), tornado->magnitude, tornado->radius});
        }
    }
}

void
Simulation::ApplySlopeForce()
{
    for (auto &&object : objects) {
        if (!object->terrain_movable) {
            continue;
        }

        auto width = (float)terrain->getTextureWidth();
        auto height = (float)terrain->getTextureHeight();

        auto robotPos = object->getPosition();
        b2Vec2 terrainPixelPos = {robotPos.x + width / 2.f, height / 2.f - robotPos.y};

        float x = terrainPixelPos.x;
        float y = terrainPixelPos.y;

        float slopeX =
            terrain->getHeight(x < width - 1.f ? x + 1.f : x, y) - terrain->getHeight(x > 0.f ? x - 1.f : x, y);
        float slopeZ =
            terrain->getHeight(x, y < height - 1.f ? y + 1.f : y) - terrain->getHeight(x, y > 0.f ? y - 1.f : y);

        if (x == 0 || x == width - 1) {
            slopeX *= 2;
        }

        if (y == 0 || y == height - 1) {
            slopeZ *= 2;
        }

        glm::vec3 slopeDir = {-slopeX * (width - 1), (width - 1), slopeZ * (height - 1)};

        if (glm::abs(slopeDir.x) > 0.1f || glm::abs(slopeDir.z) > 0.1f) {
            slopeDir *= 0.2f; // constant that seems to work
            object->addForce(b2Vec2{slopeDir.x, slopeDir.z});
        }
    }
}

void
Simulation::Step(Settings &settings)
{
    g_debugDraw.DrawImageTexture(
        terrain->getTextureID(), {0.f, 0.f}, {(float)terrain->getTextureWidth(), (float)terrain->getTextureHeight()});

    shadow_zone->draw();

    earthquake.update(m_stepCount);

    UpdateObjects();

    for (auto &&object : objectsSpawned) {
        SimulateObject(object);
    }
    objectsSpawned.clear();

    for (auto &&object : objectsDestroyed) {
        DestroyObject(object);
    }

    objectsDestroyed.clear();

    ApplySlopeForce();

    BroadcastGeneralInfo();

    Application::Step(settings);
}

void
Simulation::Keyboard(int key)
{

    if (key == GLFW_KEY_W) {
        robot->leftAccelerate = 1.f;
    } else if (key == GLFW_KEY_S) {
        robot->leftAccelerate = -1.f;
    }

    if (key == GLFW_KEY_E) {
        robot->rightAccelerate = 1.f;
    } else if (key == GLFW_KEY_D) {
        robot->rightAccelerate = -1.f;
    }
}
void
Simulation::KeyboardUp(int key)
{
    if (key == GLFW_KEY_W || key == GLFW_KEY_S) {
        robot->leftAccelerate = 0.f;
    }

    if (key == GLFW_KEY_E || key == GLFW_KEY_D) {
        robot->rightAccelerate = 0.f;
    }

    if (key == GLFW_KEY_F) {
        earthquake.trigger(50.f, 500);
    }

    if (key == GLFW_KEY_V) {
        volcano->trigger(3500.f, 500);
    }
}
void

Simulation::SimulateObject(Object *object)
{
    objects.push_back(object);
}

void
Simulation::DestroyObject(Object *object)
{
    auto it = std::find(objects.begin(), objects.end(), object);
    if (it != objects.end()) {
        objects.erase(it);
    }

    for (auto &&attachedObject : object->getAttachedObjects()) {
        auto it2 = std::find(objects.begin(), objects.end(), attachedObject);
        if (it2 != objects.end()) {
            objects.erase(it2);
        }

        m_world->DestroyBody(attachedObject->body);
        delete attachedObject;
    }

    m_world->DestroyBody(object->body);

    delete object;
}

void
Simulation::BeginContact(b2Contact *contact)
{

    b2Fixture *fixtureA = contact->GetFixtureA();
    b2Fixture *fixtureB = contact->GetFixtureB();

    auto *a = reinterpret_cast<Object *>(fixtureA->GetUserData().pointer);
    auto *b = reinterpret_cast<Object *>(fixtureB->GetUserData().pointer);

    if (auto sensor = dynamic_cast<ProximitySensor *>(a)) {
        // std::cout << "COLLIDE WITH SENSOR A " << b->name << std::endl;
        sensor->ObjectEnter(b);
    }

    if (auto sensor = dynamic_cast<ProximitySensor *>(b)) {
        // std::cout << "COLLIDE WITH SENSOR B " << a->name << std::endl;
        sensor->ObjectEnter(a);
    }
}

void
Simulation::EndContact(b2Contact *contact)
{
    b2Fixture *fixtureA = contact->GetFixtureA();
    b2Fixture *fixtureB = contact->GetFixtureB();

    auto *a = reinterpret_cast<Object *>(fixtureA->GetUserData().pointer);
    auto *b = reinterpret_cast<Object *>(fixtureB->GetUserData().pointer);

    if (auto sensor = dynamic_cast<ProximitySensor *>(a)) {
        // std::cout << "LEAVE SENSOR A " << b->name << std::endl;
        sensor->ObjectLeave(b);
    }

    if (auto sensor = dynamic_cast<ProximitySensor *>(b)) {
        // std::cout << "LEAVE SENSOR B " << a->name << std::endl;
        sensor->ObjectLeave(a);
    }
}
Robot *
Simulation::GetRobot()
{
    return robot;
}

b2World *
Simulation::GetWorld()
{
    return m_world;
}
int32
Simulation::GetStepCount()
{
    return m_stepCount;
}
void
Simulation::WakeAllObjects()
{
    for (auto &&object : objects) {
        object->body->SetAwake(true);
    }
}
Terrain *
Simulation::GetTerrain()
{
    return terrain;
}
void
Simulation::GenerateBlurredTerrain()
{
    if (std::filesystem::exists("data/lunar_received.png")) {
        // If there is a received lunar image, use this instead
        std::cout << "Using provided received lunar image." << std::endl;
        Terrain::GenerateGaussianImageFromHardEdgeImage("data/lunar_received.png", "data/lunar_blurred.png", 1.2f);
    } else {
        // Default to lunar_hard
        std::cout << "No received lunar image found, using default lunar image." << std::endl;
        Terrain::GenerateGaussianImageFromHardEdgeImage("data/lunar_hard.png", "data/lunar_blurred.png", 1.2f);
    }

    if (terrain) {
        delete terrain;
    }

    terrain = new Terrain{"data/lunar_blurred.png"};
}

std::vector<TornadoData> &
Simulation::GetTornados()
{
    return tornadoDatas;
}

std::vector<VolcanoData>
Simulation::GetVolcanoes() const
{
    std::vector<VolcanoData> vds;
    VolcanoData vd{};
    vd.magnitude = volcano->magnitude;
    vd.radius = volcano->radius;
    vd.pos = volcano->getPosition();
    vds.push_back(vd);
    return vds;
}

void
Simulation::BroadcastGeneralInfo()
{
    static unsigned int i = 0;
    i++;

    // Broadcast general info every 5 seconds
    if (i % 300 == 0) {
        nlohmann::json j = GetGeneralInfo();
        Mqtt::getInstance().send("sim/out/general", "info", j);
    }
}
void
Simulation::SimulateObjectNextFrame(Object *object)
{
    objectsSpawned.push_back(object);
}

void
Simulation::DestroyObjectNextFrame(Object *object)
{
    objectsDestroyed.push_back(object);
}

nlohmann::json
Simulation::GetGeneralInfo()
{
    nlohmann::json j;
    j["shadowFrontierX"] = shadow_zone->pos.x;
    j["shadowFrontierY"] = shadow_zone->pos.y;
    j["shadowFrontierR"] = shadow_zone->rot;
    j["satelliteImageScaleFactor"] = imageScaleFactorMultiplier;

    return j;
}
