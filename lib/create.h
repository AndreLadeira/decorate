#ifndef CREATE_H
#define CREATE_H

#include "facilities.h"

namespace onion {

template< typename solution_t >
class Creator : NonCopyable
{
public:

    Creator() = default;
    virtual ~Creator() = default;

    virtual solution_t operator()(void) = 0;
};

}


#endif // CREATE_H
