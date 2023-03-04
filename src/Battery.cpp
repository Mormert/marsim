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

float
Battery::getOCVTable(double soc){
    //leave table below for future reference, can be deleted after testing
    //float soc2[11] = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};
    float ocv[11] = {3.05, 3.66, 3.72, 3.76, 3.79, 3.83, 3.88, 3.94, 4.01, 4.09, 4.18};
    for (int i= 0; i < std::size(ocv); i++){
        ocv[i] = ocv[i] * (Voltage_/4);
    }
    int x = std::round((soc*10));
    return ocv[x];

}

//this function returns the open circuit voltage of the battery at a certain state of charge
double
Battery::getOCV(double n)
{
    double v_min = 2.75 * Voltage_/4;
    double v_max = 4.5 * Voltage_/4;
    return v_max * pow(getSoC(), 1/n) + v_min * (1 - pow(getSoC(), 1/n));
}
int
Battery::BatUpdate(double I)
{
    //I is positive when supplying energy to robot, negative when charging
    double SOC_0, SOC;
    SOC_0 = getSoC();
    current_tick_drain += I;

    //this section is still being fixed, but it is 40-60% accurate atm
    /////////////////////////////////////////////////////////////////////////
    //[1 min, 1 hr, 60 hrs] in sim per second is [3600, 60, 1] value to be multiplied by cap_
    //NOTE: THIS FUNCTION IS ONLY INTENDED TO WORK AT 60 Hz, TIME FACTOR WILL CHANGE IF NOT UPDATED AT 60Hz
    if (SOC_0 >= 0.8 && SOC_0 <= 100){
        SOC = SOC_0 - (I*(0.2*CRate_)/ (cap_ * 60));


    }else{
    if (SOC_0 <= 0.2){
        SOC = SOC_0 - (I*(2*CRate_)/ (cap_ * 60));

    }
    else{
        SOC = SOC_0 - (I*CRate_ /(cap_ * 60 ));

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
    //std::cout << SOC << std::endl;
    cbs_ = SOC * cap_;
    return 1;
}
Battery::Battery(double voltage, double cap, double res, double CRate)
    : Voltage_(voltage), cap_(cap), res_(res), CRate_(CRate) {
    cbs_ = cap_;
}
void
Battery::reset_current_tick_drain()
{
    current_tick_drain = 0;
}
float
Battery::GetCurrentTick()
{
    return current_tick_drain;
}
