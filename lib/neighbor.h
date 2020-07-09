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
    virtual std::vector<solution_t> get(const solution_t &) const = 0;
};
}

#endif // NEIGHBOR_H
