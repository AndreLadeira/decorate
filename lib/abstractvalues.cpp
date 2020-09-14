/*
#include "abstractvalues.h"

using namespace onion;

onion::AResettable::AResettable(bool locked):_locked(locked){}

void onion::AResettable::reset(){
    if (!_locked) this->doReset();
}

void onion::AResettable::hardReset(){
    this->doReset();
}

onion::Counter::Counter(unsigned start):
    Value<unsigned>(start){}

void onion::Counter::count(unsigned amount)  { this->_v += amount;}


onion::ResettableCounter::ResettableCounter(unsigned start, bool locked):
    Value<unsigned>(start),AResettable(locked){}

*/
