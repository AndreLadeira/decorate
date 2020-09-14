#include "rrga.h"
#include <vector>

using namespace std;
using namespace onion::mh;

void RRGAParametersChecker::operator()(istream &is, ParameterList &paramList){
    this->_next->operator()(is,paramList);
    const vector<string> rrgaParams = { "explorations", "intensifications" };
    for(auto p:rrgaParams){
        if ( paramList(p).str() == ""  )
            throw runtime_error("RRGAParametersChecker: mandatory parameter missing: " + p);
    }
}
