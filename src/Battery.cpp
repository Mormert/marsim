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