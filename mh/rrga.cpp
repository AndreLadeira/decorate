#include "rrga.h"
#include <vector>

using namespace std;
using namespace onion::mh;

void RRGAParametersChecker::operator()(istream &is, ParameterList &paramList) const{
    DecoratorBase::_ptr->load(is,paramList);
    const vector<string> rrgaParams = { "inner_loops", "outer_loops" };
    for(auto p:rrgaParams){
        if ( paramList.getValue(p).str() == ""  )
            throw runtime_error("RRGAParametersChecker: mandatory parameter missing: " + p);
    }
}
