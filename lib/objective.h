#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "facilities.h"
#include <vector>

namespace onion{

template< typename solution_t, typename problem_data_t, typename _cost_t = unsigned >
class Objective : public NonCopyable
{
public:

    explicit Objective(const problem_data_t& d):_data(d){}
    virtual ~Objective() = default;
    using cost_t = _cost_t;
    virtual cost_t get(const solution_t&) const = 0;

public:

    std::vector<cost_t> get(const std::vector<solution_t>& S) const {
        std::vector<cost_t> r(S.size());
        for(auto s: S ) r.push_back(this->get(s));
        return std::vector<cost_t>(r);
    }

protected:

    const problem_data_t& _data;
};
}
#endif // OBJECTIVE_H
