#ifndef BMKFCNS_H
#define BMKFCNS_H

#include <array>
#include <numeric>
#include <cmath>
#include "../lib/create.h"
#include "../lib/objective.h"
#include "../lib/neighbor.h"
#include "../lib/accept.h"
#include "../lib/stddecorators.h"
#include "../lib/random.h"

namespace onion{
namespace cops{
namespace bmf{

struct Range{
    double min = 0;
    double max = 0;
};

template <unsigned S>
using solution_t = std::array<double,S>;

using bmk_fcn_t = double (*)(double);

template <unsigned S>
using sum_fcn_t = double(*)( solution_t<S>, bmk_fcn_t );

inline double f1(double x){
    return x * x;
}

inline double f8(double x){
    return -x * std::sin( std::sqrt( std::abs(x) ) );
}
const double M_2PI = 2 * M_PI;
inline double f9(double x){
    return x*x - 10.0 * std::cos( M_2PI * x ) + 10.0;
}

template <unsigned S>
double sum_fcn( const solution_t<S>& x, bmk_fcn_t f ){

    double sum = 0;
    for (const auto &xi : x)
        sum += f(xi);
    return sum;
}


//-----------------------------------------------------------------
//                  CREATE
//-----------------------------------------------------------------
template<unsigned S>
class CreateRandom : public onion::Creator< solution_t<S> >
{
public:
    CreateRandom( Range range):
        Creator< solution_t<S> >("CreateRandom"),_range(range){}

    virtual solution_t<S> operator()(void){
        static const double wid = _range.max - _range.min;
        solution_t<S> s;

        for(auto &v : s)
            v = _range.min + wid * onion::rand01();

        return solution_t<S>(s);
    }

private:
    Range _range;
};

//-----------------------------------------------------------------
//                  OBJECTIVE
//-----------------------------------------------------------------
template<unsigned S>
using Objective = onion::Objective<solution_t<S>, void, double>;

template<unsigned S>
class bmfObjective : public Objective<S>{
public:

    bmfObjective(bmk_fcn_t f):Objective<S>("bmfObjective"),_f(f){}

    virtual typename Objective<S>::cost_type
    operator()(const solution_t<S>& s){
        return sum_fcn<S>(s,_f);
    }

private:
    bmk_fcn_t _f;
};

//-----------------------------------------------------------------
//                  NEIGHBORHOOD
//-----------------------------------------------------------------
template<unsigned S>
class RandomNeighbor : public onion::Neighbor<solution_t<S>>{
public:

    RandomNeighbor(Range range):
        Neighbor<solution_t<S>>("RandomNeighbor"),_range(range){}

    std::vector< solution_t<S> > operator()(const solution_t<S>& s)
    {
        static std::vector< solution_t<S> > N(S, solution_t<S>() );
        static const double wid = _range.max - _range.min;

        unsigned i = 0;
        for(auto &n : N){
            n = s;
            n[i] = _range.min + wid * onion::rand01();
            i++;
        }
        return std::vector< solution_t<S> >(N);
    }

private:

    Range _range;
};

using range_decay_fcn_t = Range (*)(const Range& range, unsigned loop, unsigned maxloops);

// decay linearly from initial range to 1/100th of it
Range linear_decay(const Range& range, unsigned loop, unsigned maxloops);

// exponential decay to zero, aprox 60% of the loops in a small range
Range exp_decay(const Range& range, unsigned loop, unsigned maxloops);

//smooth decay to zero. 30% of the time close to zsro
// exponential decay to zero, aprox 60% of the loops in a small range
Range cos_decay(const Range& range, unsigned loop, unsigned maxloops);

template<unsigned S>
class RandomDecreasingRangeNeighbor :
        public onion::Neighbor<solution_t<S>>,
        public onion::AResettable
{
public:

    RandomDecreasingRangeNeighbor(Range range, unsigned loops, range_decay_fcn_t decay_fcn = linear_decay):
        Neighbor<solution_t<S>>("RandomNeighbor"),_range(range),
        _maxloops(loops), _current_loop(0),_decay_fcn(decay_fcn){}

    std::vector< solution_t<S> > operator()(const solution_t<S>& s)
    {
        static std::vector< solution_t<S> > N(S, solution_t<S>() );
        const auto r = _decay_fcn(_range,_current_loop,_maxloops);
        const auto wid = r.max-r.min;

        unsigned i = 0;
        for(auto &n : N){
            n = s;
            // y = y0 +dy/dx*x, y0 = rmin-n[i], dy = wid, x = rand
            const Range localr = {   n[i] - wid < _range.min ? _range.min : n[i] + r.min,
                                    n[i] + wid > _range.max ? _range.max : n[i] + r.max
                                 };
            const auto localwid = localr.max - localr.min;
            n[i] = localr.min +  localwid * onion::rand01();
            i++;
        }

        _current_loop++;

        return std::vector< solution_t<S> >(N);
    }

    void reset(){
        _current_loop = 0;
    }

private:

    Range _range;
    const unsigned _maxloops;
    unsigned _current_loop;
    range_decay_fcn_t _decay_fcn;
};


using Accept = min::Accept<double>;
using Accept1st = min::Accept1st<double>;
using AcceptBest = min::AcceptBest<double>;
using AcceptStagCounter = min::AcceptStagCounter<double>;

template<unsigned S>
using CreatorCallsCounter = CreatorCallsCounter<solution_t<S>>;

template<unsigned S>
using ObjectiveCallsCounter = ObjectiveCallsCounter<solution_t<S>, void, double>;

template<unsigned S>
using ObjectiveRecorder = ObjectiveRecorder<solution_t<S>, void, double>;

template<unsigned S>
using Updater = min::Updater<solution_t<S>,double>;

template<unsigned S>
using UpdateRecorder = min::UpdateRecorder<solution_t<S>,double>;

template<unsigned S>
using UpdateLocalRecorder = min::UpdateLocalRecorder<solution_t<S>,double>;

template<unsigned S>
using UpdateBestSolRecorder = min::UpdateBestMTRecorder<solution_t<S>,double>;

template<unsigned S>
using UpdateStagnationCounter = min::UpdateStagnationCounter<solution_t<S>,double>;

template<unsigned S>
using UpdateImprovementMeter = min::UpdateImprovementMeter<solution_t<S>,double>;

template<unsigned S>
using UpdateResetObject = min::UpdateResetObject<solution_t<S>,double>;

template<unsigned S>
using Creator = Creator<solution_t<S>>;

template<unsigned S>
using Neighbor = Neighbor<solution_t<S>>;

}// bmf
}// cops
}// onion


#endif // BMKFCNS_H
