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

#include "Battery.h"
double
Battery::Charge(double I, double T){
    double charge = I * T;
    cbs_ += charge;
    if( cbs_ > cap_ ){
        cbs_ = cap_;
    }
    return charge;
}

//I is amps and T time in seconds
double
Battery::Supply(double I, double T){
    double discharge = I * T;
    cbs_ -= discharge;
    if ( cbs_ < 0 ){
        cbs_ = cap_;
    }
    return discharge;
}

double
Battery::getSoC(){
    return (cbs_ / cap_);
}

double
Battery::getV(double I){
    return getSoC() * cap_ / cbs_ + res_ * I;
}

//this function returns the open circuit voltage of the battery at a certain state of charge
double
Battery::getOCV(double n)
{
    double v_min = 3 * Voltage_;
    double v_max = 5 * Voltage_;
    return v_max * pow(getSoC(), 1/n) + v_min * (1 - pow(getSoC(), 1/n));
}
int
Battery::BatUpdate(double I, double n = 1)
{
    //I is positive when supplying energy to robot, negative when charging
    double SOC_0, SOC;
    //n is the number of electrons in the battery reaction, usually == 1 so don't touch unless necessary
    SOC_0 = getSoC();

    //this section is still being fixed, but it is 40-60% accurate atm
    /////////////////////////////////////////////////////////////////////////
    if (SOC_0 >= 0.8 && SOC_0 <= 100){
        SOC = SOC_0 - (I*(0.2*CRate_)/ cap_ * 3600 * n);
    }else{
    if (SOC_0 <= 0.2){
        SOC = SOC_0 - (I*(2*CRate_)/ cap_ * 3600 * n);
    }
    else{
        SOC = SOC_0 - (I*CRate_ / cap_ * 3600 * n);
    }}
    /////////////////////////////////////////////////////////////////////////
    if(SOC * cap_ > cap_){
        //when trying to charge over 100%
        return 0;
    }
    if(SOC * cap_ < 0){
        //when trying to use battery when discharged
        return 0;
    }
    cbs_ = SOC * cap_;
    return 1;
}
