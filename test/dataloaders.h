#ifndef DATALOADERS_H
#define DATALOADERS_H

#include <iostream>
#include <fstream>
#include "../lib/onionmh.h"
using namespace std;

void testDataLoaders(void)
{
    ifstream ifs;
    ifs.exceptions( istream::failbit | istream::badbit  );

    ifs.open("../onionMH/data/atsp/tsplib/br17.atsp");
    auto tsp = onion::cops::tsp_tsplibDataLoader().load( ifs );
    std::cout<< "tsp Data loaded successfully!\n";

    ifs.close();
    ifs.open("../onionMH/data/mkp/chubeas/OR5x100/OR5x100-0.25_1.dat");
    auto mkp = onion::cops::mkp_datDataLoader().load(ifs);
    std::cout<< "mkp Data loaded successfully!\n";

    ifs.close();
    ifs.open("../onionMH/data/sat/uf20-91/uf20-012.cnf");
    auto sat = onion::cops::sat_cnfDataLoader().load(ifs);
    std::cout<< "sat Data loaded successfully!\n";

    ifs.close();
}

#endif // DATALOADERS_H
