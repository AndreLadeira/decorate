#include "values.h"

using namespace onion;

AResettable::AResettable(bool locked):_locked(locked){}

AResettable::~AResettable(){}

Counter::Counter(unsigned start):
    Value<unsigned>(start){}

void Counter::count(unsigned amount)  { this->_v += amount;}

void ResettableCounter::count(unsigned amount)  {
    this->_v += amount;
    _accumulated += amount;
}

unsigned ResettableCounter::getAccumulated()
{
    return _accumulated;
}

ACounter::~ACounter()
{
}
