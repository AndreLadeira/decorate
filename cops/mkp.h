#ifndef KNAPSACK_H
#define KNAPSACK_H

#include "../lib/types.h"
#include "../lib/dataload.h"

namespace onion{
namespace cops{

using mkp_solution_t     = onion::array_of<bool>;


struct mkp_data_t
{
    mkp_data_t(unsigned dimensions, unsigned products );

    onion::matrix_of<unsigned> costs;
    onion::array_of<unsigned>  profits;
    onion::array_of<unsigned>  capacities;
};

class mkp_datDataLoader : public onion::DataLoader<mkp_data_t>
{
public:
    mkp_data_t load(std::istream& is) const;
};

}
}

#endif // KNAPSACK_H
