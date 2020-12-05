#include "values.h"

using namespace onion;

Counter::Counter(unsigned start):Value<unsigned>(start){}

void Counter::count(unsigned amount){
    this->_v += amount;
}

ResettableCounter::ResettableCounter(unsigned start):Counter(start){}
