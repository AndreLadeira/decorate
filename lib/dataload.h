#ifndef DATA_LOAD_H
#define DATA_LOAD_H

#include <istream>
#include "facilities.h"

namespace onion {

template<typename problem_data_t>
class DataLoader : public NonCopyable
{
public:
    virtual problem_data_t load(std::istream& f) const = 0;
    virtual ~DataLoader() = default;
};


}

#endif // DATA_LOAD_H
