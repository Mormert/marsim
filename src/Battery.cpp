//
// Created by ermia on 2023-01-22.
//

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