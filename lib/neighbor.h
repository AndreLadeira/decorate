#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include "facilities.h"
#include <vector>

namespace onion{
template< typename solution_t>
class Neighbor : public NonCopyable
{
public:

    virtual ~Neighbor() = default;
    virtual std::vector<solution_t> operator()(const solution_t &) = 0;
};
}

#endif // NEIGHBOR_H
