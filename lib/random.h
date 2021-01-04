#ifndef ONION_RANDOM_H
#define ONION_RANDOM_H

#include <random>
#include <chrono>

namespace onion{

using rand_num_t = decltype(std::default_random_engine()());

void reset_random_engine(unsigned seed = static_cast<unsigned>(
            std::chrono::duration_cast< std::chrono::milliseconds >(
                std::chrono::system_clock::now().time_since_epoch()).count() ));
std::default_random_engine& get_random_engine(void);
rand_num_t rand(void);
double rand01(void);
rand_num_t rand_between(rand_num_t min, rand_num_t max);

}

#endif // RANDOM_H
