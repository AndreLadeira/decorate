#include "timer.h"

using namespace onion;

void Timer::start(){
    static bool started = false;
    if (!started){
        begin = clock();
        started = true;
    }
}

Timer::Timer(bool startnow){
    if ( startnow ) start();
}

double Timer::getValue() const {
    return static_cast<double>(clock() - begin) / CLOCKS_PER_SEC;
}

void Timer::setValue(const double& v){}

void ResettableTimer::doReset(){
    begin = clock();
}
