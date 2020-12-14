#ifndef DATALOADERS_H
#define DATALOADERS_H

#include <iostream>
#include <fstream>
#include "../cops/tsp.h"
#include "../cops/mkp.h"
#include "../cops/sat.h"

using namespace std;
using namespace onion::cops;
void testDataLoaders(void)
{
    ifstream ifs;
    ifs.exceptions( istream::failbit | istream::badbit  );

    ifs.open("../onionMH/data/atsp/tsplib/br17.atsp");
    auto tsp = tsp::tsp_tsplibDataLoader()( ifs );

    std::cout<< "tsp Data loaded successfully!\n";

    ifs.close();
    ifs.open("../onionMH/data/mkp/chubeas/OR5x100/OR5x100-0.25_1.dat");
    auto mkp = mkp::mkp_datDataLoader()(ifs);
    std::cout<< "mkp Data loaded successfully!\n";

    ifs.close();
    ifs.open("../onionMH/data/sat/uf20-91/uf20-012.cnf");
    auto sat = sat::sat_cnfDataLoader()(ifs);
    std::cout<< "sat Data loaded successfully!\n";

    ifs.close();
}

#endif // DATALOADERS_H
