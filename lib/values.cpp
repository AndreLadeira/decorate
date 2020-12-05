#include "values.h"

using namespace onion;

Counter::Counter(unsigned v0):ResettableValue<unsigned>(v0){}

void Counter::count(unsigned amount){
    this->_v += amount;
}

//ResettableCounter::ResettableCounter(unsigned start):Counter(start){}
