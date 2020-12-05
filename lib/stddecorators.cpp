#include "stddecorators.h"

using namespace onion;

bool LoopTimer::operator()()
{
    static bool started     = false;
    auto        isrunning   = this->_next->operator()();

    if (isrunning){
        if (!started) {
            this->start();
            started = true;
        }
    }
    else{
        if (started) {
            this->stop(); started = false;
        }
    }

    return isrunning;
}

bool LoopRecorder::operator()()
{
    Recorder::record();

    auto running = this->_next->operator()();
    if ( !running ) Recorder::restart();

    return running;
}

bool LoopCounter::operator()()
{
    auto isrunning = this->_next->operator()();
    if ( isrunning ) Counter::count();
    return isrunning;
}
