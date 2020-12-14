#ifndef CREATE_H
#define CREATE_H

#include "facilities.h"

namespace onion {

template< typename solution_t >
class Creator : public NonCopyable, public LabeledObject
{
public:

    Creator(const char * const& name):LabeledObject(name){}
    virtual ~Creator() = default;

    virtual solution_t operator()(void) = 0;
};

}


#endif // CREATE_H
