#ifndef ONION_RANDOM_H
#define ONION_RANDOM_H

#include <random>

namespace onion{

using rand_num_t = decltype(std::default_random_engine()());

std::default_random_engine& get_random_engine(void);
rand_num_t rand(void);
double rand01(void);
rand_num_t rand_between(rand_num_t min, rand_num_t max);

}

#endif // RANDOM_H
