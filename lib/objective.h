#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "facilities.h"
#include <vector>

namespace onion{

template< typename solution_t,
          typename problem_data_t,
          typename cost_t = unsigned >
class Objective : public NonCopyable, public LabeledObject
{
public:

    explicit Objective(const problem_data_t * const d, const char* const& name ):
        LabeledObject(name),_data(d){}

    explicit Objective( const char* const& name ):
        LabeledObject(name),_data( nullptr ){}

    virtual ~Objective() = default;
    using cost_type = cost_t;

    virtual cost_type operator()(const solution_t&) = 0;

    virtual std::vector<cost_type> operator()(const std::vector<solution_t>& S){
        std::vector<cost_type> costs;
        costs.reserve(S.size());

        for( const auto& s : S)
            costs.push_back( (*this)(s) );

        return std::vector<cost_type>(costs);
    }

protected:

    const problem_data_t * const _data;
};

//namespace  delta{

//template< typename solution_t,
//          typename transformation_t,
//          typename problem_data_t,
//          typename cost_t = unsigned >
//class Objective
//{
//public:

//    explicit Objective(const problem_data_t& d = problem_data_t() ):_data(d){}
//    virtual ~Objective() = default;
//    using cost_type = cost_t;

//    virtual cost_type operator()(const solution_t&, const transformation_t& ) = 0;

//protected:

//    const problem_data_t& _data;
//};

//} // delta


} // namespace onion
#endif // OBJECTIVE_H
