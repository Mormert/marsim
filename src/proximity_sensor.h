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

#ifndef MARSIM_PROXIMITY_SENSOR_H
#define MARSIM_PROXIMITY_SENSOR_H

#include "object.h"
#include <vector>

class ProximitySensor : public Object
{
public:
    
    ProximitySensor(Simulation* simulation, b2Vec2 pos, float radius, bool isDynamic = false, bool updatedBySim = true);

    void ObjectEnter(Object* other);

    void ObjectLeave(Object* other);

    void setRadius(float r);

    void update() override;

    std::vector<Object*> getObjectsInside();

    bool shouldTransmitMqtt = true;

protected:
    explicit ProximitySensor(Simulation* simulation);

    virtual void OnObjectEnter(Object * o){};
    virtual void OnObjectLeave(Object * o){};

    std::vector<Object*> objects_inside;

    float radius{15.f};

    int updateFrequency = 20;

    void MoveToMiddleMouseButtonPressPosition();

};

#endif // MARSIM_PROXIMITY_SENSOR_H
