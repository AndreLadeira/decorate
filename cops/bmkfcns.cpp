#include "bmkfcns.h"
#include "../lib/random.h"

using namespace onion::cops::bmf;

namespace{
const unsigned RES_RANGE_FRAC = 500;
}

Range onion::cops::bmf::linear_decay(const Range &range, unsigned loop, unsigned maxloops){

    static const auto basemin = range.min / RES_RANGE_FRAC;
    static const auto basemax = range.max / RES_RANGE_FRAC;

    return
    {
        range.min - (range.min - basemin)/maxloops * loop,
        range.max - (range.max - basemax)/maxloops * loop,
    };
}

Range onion::cops::bmf::exp_decay(const Range &range, unsigned loop, unsigned maxloops){

    static const auto basemin = range.min / RES_RANGE_FRAC;
    static const auto basemax = range.max / RES_RANGE_FRAC;

    double f = 1 - std::tanh(5*loop/maxloops);

    if (loop > 400){
        f *= 1.0;
    }

    return
    { basemin + range.min * f,basemax + range.max * f};
}

Range onion::cops::bmf::cos_decay(const Range &range, unsigned loop, unsigned maxloops){

    static const auto basemin = range.min / RES_RANGE_FRAC;
    static const auto basemax = range.max / RES_RANGE_FRAC;
    static const auto p = M_PI / 2 / maxloops;

    double f = std::pow( std::cos( p * loop ), 4.0 );

    return
    { basemin + range.min * f,basemax + range.max * f};
}
