#include "mkp.h"
using namespace std;
using namespace onion::cops;

mkp_data_t::mkp_data_t(unsigned dimensions, unsigned products):
    costs(dimensions,products),profits(products),capacities(dimensions){}

mkp_data_t mkp_datDataLoader::load(std::istream& is) const
{
    unsigned products = 0;
    unsigned dimensions = 0;
    unsigned useless = 0;

    is >> products >> dimensions >> useless;

    mkp_data_t data(dimensions, products );

    for(unsigned i = 0; i < products; ++i)
      is >> data.profits[i];

    for(unsigned i = 0; i < dimensions; ++i)
      for(unsigned j = 0; j < products; ++j)
          is >> data.costs[i][j];

    for(unsigned i = 0; i < dimensions; ++i)
      is >> data.capacities[i];

    return data;
}


