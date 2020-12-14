#include "sat.h"

#include <string>
#include <sstream>

using namespace std;
using namespace onion::cops::sat;

sat_data_t sat_cnfDataLoader::operator()(istream& is)
try
{
    string s;
    while( !is.eof() ){

        getline(is,s);

        if ( !s.size() || s.at(0) == 'c' ) continue;

        if ( s.at(0) == '%' )
        {
            s = "x";
        }

        if ( s.at(0) == 'p' ){

            std::stringstream ss(s);
            std::string type;
            unsigned num_vars = 0;
            unsigned num_clauses = 0;

            ss >> type >> type >> num_vars >> num_clauses;
            if ( type != "cnf" )
                throw std::runtime_error("sat_cnfDataLoader: not a .cnf format file");

            sat_data_t data(num_clauses);
            int xi;
            unsigned clauseNumber = 0;

            while( clauseNumber < num_clauses ){
                sat_clause clause;
                do{
                   is >> xi;
                   if ( xi ) clause.push_back(xi);
                   else data[clauseNumber++] = clause;
                }
                while(xi);
            }
            return data;
        }
    }
    throw std::runtime_error("sat_cnfDataLoader: corrupt data file");
}
catch(istream::failure& ){
    throw std::runtime_error("sat_cnfDataLoader: corrupt data file");
}
