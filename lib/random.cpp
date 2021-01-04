#include "random.h"
#include <chrono>
#include <limits>

using namespace onion;
using namespace std;
using namespace std::chrono;
namespace{
static default_random_engine e( static_cast<unsigned>(
                                    duration_cast< milliseconds >(
                                        system_clock::now().time_since_epoch()).count() ) );

static uniform_real_distribution<double> uniform01(0,1);

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
   return uniform01(e);
}

rand_num_t onion::rand_between(rand_num_t min, rand_num_t max)
{
    auto span = max-min+1;
    return e() % span + min;
}
