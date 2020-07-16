#include "facilities.h"

using namespace onion;

Timer::Timer(bool startnow, bool resetable):
    Value<double>(resetable)
{ if (startnow) start();}

void Timer::start()
{
    begin = clock();
}

void Timer::reset()
{
    start();
}
void Timer::hardReset()
{
    reset();
}

double Timer::getValue() const
{
    return static_cast<double>(clock() - begin) / CLOCKS_PER_SEC;
}

void Counter::count(unsigned amount)  { this->_v += amount;}
