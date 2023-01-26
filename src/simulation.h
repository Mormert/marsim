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
#include "terrain.h"

class Object;
class Robot;
class Volcano;

class Simulation : public Application
{
public:
    Simulation();

    ~Simulation() override;

    void UpdateObjects();

    void WakeAllObjects();

    void ApplySlopeForce();

    void Step(Settings &settings) override;

    void Keyboard(int key) override;

    void KeyboardUp(int key) override;

    void SimulateObject(Object *object);

    void DestroyObject(Object *object);

    void GenerateBlurredTerrain();

    b2World* GetWorld();

    Terrain* GetTerrain();

    static Simulation *Create();

    void BeginContact(b2Contact *contact) override;

    void EndContact(b2Contact *contact) override;

    int32 GetStepCount();

    Robot* GetRobot();

    Earthquake earthquake;

    Volcano* volcano;

private:
    Robot* robot;
    Terrain* terrain{nullptr};
    std::vector<Object *> objects;
};

#endif
