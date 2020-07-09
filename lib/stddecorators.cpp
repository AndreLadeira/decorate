#include "stddecorators.h"

using namespace onion;

bool LoopCallsCounter::running()
{
    auto isrunning = DecoratorBase::_ptr->running();
    if ( isrunning ) this->count();
    return isrunning;
}
