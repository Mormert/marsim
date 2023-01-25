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

#include "earthquake.h"
#include "simulation.h"

#include <random>

Earthquake::Earthquake(b2World *world, Simulation *sim)
{
    this->world = world;
    this->simulation = sim;
}

void
Earthquake::trigger(float magnitude, int steps)
{
    continueUntil = currentStep + steps;
    this->magnitude = magnitude;
    simulation->WakeAllObjects();
}

void
Earthquake::update(int step)
{
    currentStep = step;

    if (magnitude == 0.f) {
        return;
    }

    if (currentStep >= continueUntil) {
        magnitude = 0.f;
        world->SetGravity(b2Vec2{0.f, 0.f});
    }

    if (step % 10 == 0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> v(-1.f, 1.f);

        world->SetGravity(b2Vec2{(float)v(gen) * magnitude, (float)v(gen) * magnitude});
    }
}
