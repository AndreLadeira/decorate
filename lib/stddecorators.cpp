#include "stddecorators.h"

using namespace onion;

bool LoopCallsCounter::operator()()
{
    auto isrunning = this->_next->operator()();
    if ( isrunning ) {
        ResettableCounter::count();
    }
    return isrunning;
}

bool LoopRecorder::operator()()
{
    auto isrunning = this->_next->operator()();
    if ( isrunning ) this->record();
    return isrunning;
}
