#ifndef UPDATE_H
#define UPDATE_H

#include "facilities.h"

namespace onion{

template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c
          >
class Updater : public NonCopyable, public LabeledObject
{
public:

    Updater(const char * const & name = "Updater" ):LabeledObject(name){}
    virtual ~Updater() = default;

    virtual bool operator()(solution_t& bestSoFar, cost_t& bsfCost,
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

template< typename solution_t,typename cost_t = unsigned >
using Updater = Updater<solution_t, cost_t, Compare<cost_t>::greater >;

}

namespace min{

template< typename solution_t,typename cost_t = unsigned>
using Updater = Updater<solution_t,cost_t,Compare<cost_t>::less >;

}

//-----------------------------------------------------------------------------
//                              Deltas
//-----------------------------------------------------------------------------
//namespace delta{

//template< typename solution_t,
//          typename Compare<int>::compare_fcn_t c
//          >
//class Updater : public NonCopyable
//{
//public:

//    Updater() = default;
//    virtual ~Updater() = default;

//    virtual bool operator()(solution_t& bestSoFar, const solution_t& candidate, int candidateDelta )  {
//        if ( _compare( candidateDelta, 0) ){
//            bestSoFar = candidate;
//            return true;
//        }
//        return false;
//    }
//private:

//    typename Compare<int>::compare_fcn_t _compare = c;
//};

//namespace max{

//template< typename solution_t>
//using Updater = Updater<solution_t,Compare<int>::greater>;

//}

//namespace min{

//template< typename solution_t>
//using Updater = Updater<solution_t,Compare<int>::less>;


//}
//} // delta

} // onion

#endif // UPDATE_H
