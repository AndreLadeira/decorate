#ifndef ACCEPT_H
#define ACCEPT_H

#include "facilities.h"
#include <stdexcept>
#include <vector>
#include <limits>

namespace onion {

struct AcceptResult
{
public:
    explicit AcceptResult(unsigned index):_accepted(true),_index(index){}
    AcceptResult() = default;

    operator bool() const { return _accepted; }
    unsigned index() const{
        if (_accepted) return _index;
        throw std::runtime_error("AcceptResult: no results.");
        //return _index;
    }
    operator unsigned() const { return index(); }

private:

    bool _accepted = false;
    unsigned _index = 0;

};

template< typename cost_t,
          typename Compare<cost_t>::compare_fcn_t compare>
class Accept : public NonCopyable, public LabeledObject
{
public:
    Accept(const char * const& name):LabeledObject(name){}
    virtual ~Accept() = default;
    virtual  AcceptResult operator()(const cost_t&,const std::vector<cost_t>&) = 0;

};

template< typename cost_t,
          typename Compare<cost_t>::compare_fcn_t compare>
class Accept1st : public Accept<cost_t,compare >
{
public:

    Accept1st():Accept<cost_t,compare >("Accept1st"){}

    virtual  AcceptResult operator()(const cost_t& best_so_far,
                              const std::vector<cost_t>& results){
        for(unsigned i = 0; i < results.size(); ++i){
            if ( compare(results[i],best_so_far) )
                return AcceptResult(i);
        }
        return AcceptResult();
    }
};

template< typename cost_t,
          typename Compare<cost_t>::compare_fcn_t compare>
class AcceptBest : public Accept<  cost_t, compare >
{
public:

    AcceptBest():Accept<cost_t,compare >("AcceptBest"){}

    virtual  AcceptResult operator()(const cost_t& best_so_far,
                              const std::vector<cost_t>& results){

        const auto  unchanged   = std::numeric_limits<unsigned>::max();
        auto        best_pos    = unchanged;
        auto        best_result = best_so_far;

        for(unsigned i = 0; i < results.size(); ++i)
            if ( compare(results[i],best_result) ) {
                best_pos = i;
                best_result = results[i];
            }

        if ( best_pos != unchanged && compare(best_result,best_so_far) )
            return AcceptResult(best_pos);
        else
            return AcceptResult();
    }
};

namespace max{

template< typename cost_t>
using Accept = Accept< cost_t, Compare<cost_t>::greater >;

template< typename cost_t>
using Accept1st = Accept1st<cost_t,Compare<cost_t>::greater>;

template< typename cost_t>
using AcceptBest = AcceptBest<cost_t,Compare<cost_t>::greater>;
}
namespace min{

template< typename cost_t>
using Accept = Accept< cost_t, Compare<cost_t>::less >;

template< typename cost_t>
using Accept1st = Accept1st<cost_t,Compare<cost_t>::less>;

template< typename cost_t>
using AcceptBest = AcceptBest<cost_t,Compare<cost_t>::less>;
}

//-----------------------------------------------------------------
//                  DELTA VERSIONS
//-----------------------------------------------------------------

//namespace delta{

//template< typename delta_t,
//          typename Compare<delta_t>::compare_fcn_t compare>
//class Accept : public NonCopyable
//{
//protected:

//    Accept() = default;
//    virtual ~Accept() = default;

//    virtual  AcceptResult operator()(const std::vector<delta_t>&) const = 0;

//};

//template< typename delta_t,
//          typename Compare<delta_t>::compare_fcn_t compare>
//class Accept1st : public Accept< int, compare >
//{
//public:

//    virtual  AcceptResult operator()( const std::vector<delta_t>& delta ) const {
//        for(unsigned i = 0; i < delta.size(); ++i){
//            if ( compare( delta[i], 0 ) ) return AcceptResult(i);
//        }
//        return AcceptResult();
//    }
//};

//template< typename delta_t,
//          typename Compare<delta_t>::compare_fcn_t compare>
//class AcceptBest : public Accept<int,compare>
//{
//public:

//    virtual  AcceptResult operator()( const std::vector<delta_t>& delta ) const {

//        const auto unchanged = std::numeric_limits<unsigned>::max();
//        unsigned pos = unchanged;
//        int best_delta = delta[0];

//        for(unsigned i = 1; i < delta.size(); ++i)
//            if ( compare( delta[i],best_delta ) ){
//                best_delta = delta[i];
//                pos = i;
//            }
//        if ( pos != unchanged && compare(best_delta,0))
//            return AcceptResult(pos);
//        else
//            return AcceptResult();
//    }
//};

//namespace max{

//template< typename delta_t = int >
//using Accept = Accept< delta_t, Compare<delta_t>::greater >;

//template< typename delta_t = int >
//using Accept1st = Accept1st< delta_t, Compare<delta_t>::greater >;

//template< typename delta_t = int >
//using AcceptBest = AcceptBest< delta_t, Compare<delta_t>::greater >;
//}

//namespace min{

//template< typename delta_t = int >
//using Accept = Accept< delta_t, Compare<delta_t>::less >;

//template< typename delta_t = int >
//using Accept1st = Accept1st< delta_t, Compare<delta_t>::less >;

//template< typename delta_t = int >
//using AcceptBest = AcceptBest< delta_t, Compare<delta_t>::less >;

//}// min
//}// namespace delta

}// namespace onion
//

#endif // ACCEPT_H
