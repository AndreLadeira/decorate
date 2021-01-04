#include "rrga.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace onion::mh;

void RRGAParametersChecker::operator()(istream &is, ParameterList &paramList){
    this->_next->operator()(is,paramList);
    const vector<string> rrgaParams = { "file_name", "explorations", "intensifications"};
    for(auto p:rrgaParams){
        if ( paramList(p).str() == ""  )
            cout<< "[W] - [RRGAParametersChecker]: critical parameter missing: " << p << endl;
    }
}
