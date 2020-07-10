#include "facilities.h"

using namespace onion;

Timer::Timer(bool startnow){ if (startnow) start();}

void Timer::start()
{
    begin = clock();
}

void Timer::reset()
{
    start();
}

double Timer::getValue() const
{
    return static_cast<double>(clock() - begin) / CLOCKS_PER_SEC;
}

void Counter::reset(){
    if ( _resetable )
        Value<unsigned>::reset();
}
