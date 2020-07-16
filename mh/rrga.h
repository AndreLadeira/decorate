#ifndef RRGA_H
#define RRGA_H

#include "../lib/parameters.h"
#include "../lib/onion.h"

namespace onion {
namespace mh{

class RRGAParametersChecker:
        public ParameterLoader,
        public OnionLayer<ParameterLoader>
{
public:

    RRGAParametersChecker( OnionLayer<ParameterLoader>::core_ptr_t next):
        OnionLayer<ParameterLoader>(next){}

    virtual void operator()(std::istream& is, ParameterList& paramList);
    virtual ~RRGAParametersChecker() = default;
};

}

}

#endif // RRGA_H
