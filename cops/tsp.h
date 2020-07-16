#ifndef ATSP_H
#define ATSP_H

#include "../lib/dataload.h"
#include "../lib/create.h"
#include "../lib/types.h"
#include "../lib/neighbor.h"
#include "../lib/objective.h"
#include "../lib/accept.h"
#include "../lib/update.h"
#include "../lib/stddecorators.h"


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
   tsp_problem_data_t operator()(std::istream& is);
};

namespace path{

class CreateRandom : public onion::Creator< path_t >
{
public:
    CreateRandom(size_t sz):_size(sz){}
    virtual path_t operator()(void);
private:
    size_t _size;
};

using AbstractObjective = onion::Objective<path_t,tsp_problem_data_t>;

class Objective : public AbstractObjective{
public:

    Objective(const tsp_problem_data_t& d);
    virtual AbstractObjective::cost_type operator()(const path_t&);
};

std::ostream& operator<<(std::ostream &os, const path_t &path);

class _2optSingle : public onion::Neighbor<path_t>{
public:
    _2optSingle(unsigned length=0);
    std::vector<path_t> operator()(const path_t& path);
private:
    unsigned _length;
};

class _2optAll : public onion::Neighbor<path_t>{
public:
    _2optAll(unsigned length = 0);
    std::vector<path_t> operator()(const path_t& path);
private:
    unsigned _length;
};

class MaskReinsert : public onion::Neighbor<path_t>{
public:
    MaskReinsert(unsigned length = 0):_length(length){}
    std::vector<path_t> operator()(const path_t& path);
private:
    unsigned _length;
};

using Updater = min::Updater<tsp::path_t>;
using Creator = Creator<path_t>;
using Neighbor = Neighbor<path_t>;
using CreatorCallsCounter = CreatorCallsCounter<tsp::path_t>;
using ObjectiveCallsCounter = ObjectiveCallsCounter<tsp::path_t, tsp::tsp_problem_data_t>;

}
namespace bitmatrix{

class CreateRandom : public onion::Creator< bitmatrix_t >
{
public:
    CreateRandom(size_t sz):_size(sz){}
    virtual bitmatrix_t operator()(void);
private:
    size_t _size;
};

using ObjectiveBase = onion::Objective<bitmatrix_t,tsp_problem_data_t>;

class Objective : public ObjectiveBase{
public:

    Objective(const tsp_problem_data_t& d);
    using cost_t = ObjectiveBase::cost_type;
    virtual cost_t get(const bitmatrix_t&);
};

std::ostream& operator<<(std::ostream &os, const bitmatrix_t &path);

}

// matrix <-> path conversions
path_t to_path(const bitmatrix_t&);
bitmatrix_t to_bitmatrix(const path_t&);

using Accept1st = min::Accept1st<>;
using AcceptBest = min::AcceptBest<>;


}}}// namespace onion::cops::tsp


#endif // ATSP_H
