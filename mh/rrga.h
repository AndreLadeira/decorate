#ifndef RRGA_H
#define RRGA_H

#include "../lib/parameters.h"
#include "../lib/decorator.h"

namespace onion {
namespace mh{

class RRGAParametersChecker:
        public ParameterLoader,
        public Decorator<ParameterLoader>
{
    using DecoratorBase = Decorator< ParameterLoader >;
    friend class Decorator<ParameterLoader>;

    RRGAParametersChecker( typename DecoratorBase::ptr_t ptr):
        onion::Decorator<ParameterLoader>(ptr){}

    virtual void operator()(std::istream& is, ParameterList& paramList) const;

public:

    virtual ~RRGAParametersChecker() = default;
};

}

}

#endif // RRGA_H
