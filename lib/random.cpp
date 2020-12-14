#include "random.h"
#include <chrono>
#include <limits>

using namespace onion;
using namespace std;

namespace{
static default_random_engine e(static_cast<unsigned int>(clock()));
}

void onion::reset_random_engine(unsigned int seed)
{
   e.seed( seed );
}

default_random_engine& onion::get_random_engine()
{
    return e;
}


rand_num_t onion::rand(void)
{
    return e();
}

double onion::rand01(void)
{
    static auto max = numeric_limits<rand_num_t>::max();
    return static_cast<double>(e()) / max;

}

rand_num_t onion::rand_between(rand_num_t min, rand_num_t max)
{
    auto span = max-min+1;
    return e() % span + min;
}
