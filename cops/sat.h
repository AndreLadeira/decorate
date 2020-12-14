#ifndef SAT_H
#define SAT_H

#include "../lib/types.h"
#include "../lib/dataload.h"
#include <vector>

namespace onion{
namespace cops{
namespace sat{

using sat_solution_t = onion::array_of<bool>;
using sat_clause = onion::array_of<signed>;
using sat_data_t = onion::array_of<sat_clause>;

class sat_cnfDataLoader : public onion::DataLoader< sat_data_t >
{
public:
    sat_data_t operator()(std::istream& is);
};

}
}
}
#endif // SAT_H
