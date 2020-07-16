#ifndef DATA_LOAD_H
#define DATA_LOAD_H

#include <istream>
#include "facilities.h"

namespace onion {

template<typename problem_data_t>
class DataLoader : public NonCopyable
{
public:
    virtual problem_data_t operator()(std::istream& f) = 0;
    virtual ~DataLoader() = default;
};


}

#endif // DATA_LOAD_H
