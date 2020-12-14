#include "stddecorators.h"

using namespace onion;

bool LoopTimer::operator()()
{
    auto running = this->_next->operator()();
    Timer::start();
    return running;
}

bool LoopRecorder::operator()()
{
    Recorder::record();
    auto running = this->_next->operator()();
    if ( !running )
        Recorder::restart();
    return running;
}
bool LoopResetObject::operator()()
{
    auto running = this->_next->operator()();
    if ( !running ) _object.reset();
    return running;
}

bool LoopCounter::operator()()
{
    auto isrunning = this->_next->operator()();
    if ( isrunning ) Counter::count();
    return isrunning;
}


