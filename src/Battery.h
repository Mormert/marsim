//
// Created by ermia on 2023-01-22.
//

#ifndef MARSIM_BATTERY_H
#define MARSIM_BATTERY_H

class Battery
{
private:
    double cap_;
    double res_;
    double cbs_ = 0.0;
public:
    Battery(double cap, double res): cap_(cap), res_(res) { }
    double Charge(double I, double T);
    double Supply(double I, double T);
    double getSoC();
    double getV(double I);
};

#endif // MARSIM_BATTERY_H
