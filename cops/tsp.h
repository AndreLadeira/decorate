#ifndef ATSP_H
#define ATSP_H

#include "../lib/dataload.h"
#include "../lib/create.h"
#include "../lib/types.h"
#include "../lib/neighbor.h"
#include "../lib/objective.h"
#include "../lib/accept.h"
#include "../lib/update.h"


#include <map>
#include <vector>

namespace onion{
namespace cops{
namespace tsp{

using path_t        = onion::array_of<unsigned>;
using bitmatrix_t   = onion::matrix_of<bool>;
using tsp_problem_data_t = onion::matrix_of<unsigned>;

class tsp_tsplibDataLoader : public onion::DataLoader<tsp_problem_data_t>
{
public:
   tsp_problem_data_t load(std::istream& is) const;
};

namespace path{
class CreateRandom : public onion::Creator< path_t >
{
public:
    CreateRandom(size_t sz):_size(sz){}
    virtual path_t create(void);
private:
    size_t _size;
};

class Objective : public onion::Objective<path_t,tsp_problem_data_t>{
public:

    Objective(const tsp_problem_data_t& d);
    using cost_t = onion::Objective<path_t,tsp_problem_data_t>::cost_t;
    virtual cost_t get(const path_t& p) const;

};
}
namespace bitmatrix{
class CreateRandom : public onion::Creator< bitmatrix_t >
{
public:
    CreateRandom(size_t sz):_size(sz){}
    virtual bitmatrix_t create(void);
private:
    size_t _size;
};
}

std::ostream& operator<<(std::ostream &os, const path_t &path);

// matrix <-> path conversions
path_t to_path(const bitmatrix_t&);
bitmatrix_t to_bitmatrix(const path_t&);

class _2optSingle : public onion::Neighbor<path_t>{
public:
    _2optSingle(unsigned length=0);
    std::vector<path_t> get(const path_t& path) const;
private:
    unsigned _length;
};

class _2optAll : public onion::Neighbor<path_t>{
public:
    _2optAll(unsigned length);
    std::vector<path_t> get(const path_t& path) const;
private:
    unsigned _length;
};

using Accept1st = min::Accept1st<>;

namespace path{

using Updater = min::Updater<tsp::path_t>;

}


}}}// namespace onion::cops::tsp


#endif // ATSP_H
