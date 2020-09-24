#include "recorder.h"
using namespace onion;

__Track::__Track(std::string name):_name(name){}

std::string __Track::getName() const
{
    return this->_name;
}
