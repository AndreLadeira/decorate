#include "timer.h"

using namespace onion;

void onion::Timer::start()
{
    _begin = clock();
}

void Timer::stop()
{
    _end = clock();
}

onion::Timer::Timer(bool startnow):_begin(0),_end(0)
{
    if ( startnow ) start();
}

double onion::Timer::getValue() const
{
    if ( _begin != 0 ) return static_cast<double>(clock() - _begin) / CLOCKS_PER_SEC;
    else return double(0);
}

void onion::ResettableTimer::doReset()
{
    start();
}
