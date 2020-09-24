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

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

void Timer::setValue(const double& v){}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

void ResettableTimer::doReset(){
    begin = clock();
}
