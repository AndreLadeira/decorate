#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "facilities.h"
#include <vector>

namespace onion{

template< typename solution_t,
          typename problem_data_t,
          typename _cost_t = unsigned >
class Objective : public NonCopyable
{
public:

    explicit Objective(const problem_data_t& d = problem_data_t() ):_data(d){}
    virtual ~Objective() = default;
    using cost_t = _cost_t;

    virtual std::vector<cost_t> get(const std::vector<solution_t>& S) = 0;
    virtual cost_t get(const solution_t&) = 0;

protected:

    const problem_data_t& _data;
};
}
#endif // OBJECTIVE_H
