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
    }
    operator unsigned() const { return index(); }

private:

    bool _accepted = false;
    unsigned _index = 0;

};

template< typename cost_t,
          typename Compare<cost_t>::compare_fcn_t compare>
class Accept : public NonCopyable
{
protected:

    Accept() = default;
    virtual ~Accept() = default;

    virtual  AcceptResult operator()(const cost_t&,
                              const std::vector<cost_t>&) const = 0;

};

template< typename cost_t,
          typename Compare<cost_t>::compare_fcn_t compare>
class Accept1st : public Accept<  cost_t, compare >
{
public:

    virtual  AcceptResult operator()(const cost_t& best_so_far,
                              const std::vector<cost_t>& results) const {
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

    virtual  AcceptResult operator()(const cost_t& best_so_far,
                              const std::vector<cost_t>& results) const {

        const auto unchanged = std::numeric_limits<unsigned>::max();
        unsigned best_pos = unchanged;

        for(unsigned i = 0; i < results.size(); ++i)
            if ( compare(results[i],best_so_far) ) best_pos = i;

        if ( best_pos != unchanged )
            return AcceptResult(best_pos);
        else
            return AcceptResult();
    }
};

namespace max{

template< typename cost_t = unsigned>
using Accept1st = Accept1st<cost_t,Compare<cost_t>::greater>;
template< typename cost_t = unsigned>
using AcceptBest = AcceptBest<cost_t,Compare<cost_t>::greater>;

}
namespace min{

template< typename cost_t = unsigned>
using Accept1st = Accept1st<cost_t,Compare<cost_t>::less>;
template< typename cost_t = unsigned>
using AcceptBest = AcceptBest<cost_t,Compare<cost_t>::less>;

}

}

#endif // ACCEPT_H
