#ifndef UPDATE_H
#define UPDATE_H

#include "facilities.h"

namespace onion{

template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c
          >
class Updater : public NonCopyable
{
public:

    Updater() = default;
    virtual ~Updater() = default;

    virtual bool update(solution_t& bestSoFar, cost_t& bsfCost,
                            const solution_t& candidate, const cost_t candidateCost )  {
        if ( _compare(candidateCost, bsfCost) ){
            bsfCost = candidateCost;
            bestSoFar = candidate;
            return true;
        }
        return false;
    }
private:

    typename Compare<cost_t>::compare_fcn_t _compare = c;
};

namespace max{

template< typename solution_t,typename cost_t = unsigned>
using Updater = Updater<solution_t,cost_t,Compare<cost_t>::greater>;

}
namespace min{

template< typename solution_t,typename cost_t = unsigned>
using Updater = Updater<solution_t,cost_t,Compare<cost_t>::less>;

}
}

#endif // UPDATE_H
