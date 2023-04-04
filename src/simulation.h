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

#ifndef SIMULATION_H
#define SIMULATION_H

#include "earthquake.h"
#include "framework/application.h"
#include "json.hpp"
#include "terrain.h"

#include <GLFW/glfw3.h>

class Object;
class Robot;
class Volcano;
class ShadowZone;

struct ObjectSetup
{
    b2Vec2 position;
    std::string object;
    float radius = -1.f;
};

struct SimulationSetup {
    int simulationSeed{1337};
    float robotX{0.f}, robotY{0.f}, robotR{0.f};
    std::vector<ObjectSetup> objectSetups;
    unsigned int stonesAmount{2000};
    unsigned int aliensAmount{20};
   // unsigned int proximitySensorsAmount{20};
    unsigned int frictionZonesAmount{20};
    unsigned int tornadoesAmount{20};
    unsigned int windSensorsAmount{15};
    unsigned int seismicSensorsAmount{15};
    unsigned int tempSensorsAmount{15};
    float shadowFrontierX{250.f}, shadowFrontierY{0.f}, shadowFrontierR{45.f};
    float satelliteImageScaleFactor{1.f};
    float satelliteImageScaleFactorMultiplierMin{0.7f};
    float satelliteImageScaleFactorMultiplierMax{2.0f};
    std::string satelliteImagePath{"data/lunar_image.png"};
    float objectGenerationMinX{-369.f};
    float objectGenerationMaxX{369.f};
    float objectGenerationMinY{-205.f};
    float objectGenerationMaxY{205.f};
};

struct TornadoData {
    b2Vec2 pos;
    float magnitude;
    float radius;
};

struct VolcanoData {
    b2Vec2 pos;
    float magnitude;
    float radius;
};

struct AlienData {
    b2Vec2 pos;
};

class Simulation : public Application
{
public:
    Simulation(const SimulationSetup &setup);

    ~Simulation() override;

    void UpdateObjects();

    void WakeAllObjects();

    void ApplySlopeForce();

    void Step(Settings &settings) override;

    void Keyboard(int key) override;

    void KeyboardUp(int key) override;

    void SimulateObjectNextFrame(Object* object);

    void DestroyObjectNextFrame(Object* object);

    void GenerateBlurredTerrain();

    void BroadcastGeneralInfo();

    std::vector<TornadoData> &GetTornados();

    std::vector<AlienData> &GetAliens();

    [[nodiscard]] std::vector<VolcanoData> GetVolcanoes() const;

    b2World *GetWorld();

    Terrain *GetTerrain();

    static Simulation *Create(const std::string& initJson = "");

    void BeginContact(b2Contact *contact) override;

    void EndContact(b2Contact *contact) override;

    int32 GetStepCount();

    Robot *GetRobot();

    Earthquake earthquake;

    Volcano *volcano{};

    ShadowZone *shadow_zone{};

    SimulationSetup setup;

    static inline GLFWwindow* window;

    static inline Camera* camera;

private:

    void AddObjectFromJson(ObjectSetup& os);

    void SimulateObject(Object *object);

    void DestroyObject(Object *object);

    nlohmann::json GetGeneralInfo();

    std::vector<TornadoData> tornadoDatas;
    std::vector<VolcanoData> volcanoDatas;
    std::vector<AlienData> alienDatas;

    Robot *robot;
    Terrain *terrain{nullptr};
    std::vector<Object *> objects;

    // Cleared every frame
    std::vector<Object*> objectsSpawned;
    std::vector<Object*> objectsDestroyed;

    float imageScaleFactorMultiplier;
};

#endif
