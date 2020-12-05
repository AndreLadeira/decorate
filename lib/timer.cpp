#include "timer.h"

using namespace onion;

Timer::Timer():_started(false),_begin(0),_end(0){}

double Timer::getValue() const {
    if ( _started )
        return static_cast<double>(clock() - _begin) / CLOCKS_PER_SEC;
    else
        return static_cast<double>(_end - _begin) / CLOCKS_PER_SEC;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

void Timer::setValue(const double &v){}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

bool Timer::started() const
{
    return _started;
}

void Timer::start(){
    _begin = clock();
    _started = true;
}
void Timer::stop()
{
    _end = clock();
    _started = false;
}
void Timer::reset(){
    if ( _started ) _begin = clock();
}


