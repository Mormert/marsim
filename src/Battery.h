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

#ifndef MARSIM_BATTERY_H
#define MARSIM_BATTERY_H

#include <math.h>
#include <iostream>

class Battery
{
private:
    double cap_;
    double res_;
    double cbs_ = 0.0;
    double CRate_;
    double Voltage_;
    float current_tick_drain;

public:
    Battery(double voltage, double cap, double res, double CRate);
    double Charge(double I, double T);
    double Supply(double I, double T);
    double getSoC();
    float getOCVTable(double soc);

    double getOCV(double n);
    int BatUpdate(double I);
    void reset_current_tick_drain();
    float GetCurrentTick();

};

#endif // MARSIM_BATTERY_H
