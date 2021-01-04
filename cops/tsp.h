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

using path_t                = onion::array_of<unsigned>;
using bitmatrix_t           = onion::matrix_of<bool>;
using problem_data_t        = onion::matrix_of<unsigned>;

class tsplibDataLoader : public onion::DataLoader<problem_data_t>
{
public:
   problem_data_t operator()(std::istream& is);
};

namespace path{

//-----------------------------------------------------------------
//                  CREATE
//-----------------------------------------------------------------

class CreateRandom : public onion::Creator< path_t >
{
public:
    CreateRandom(const problem_data_t& d):
        Creator< path_t >("CreateRandom"),_data(d){}
    virtual path_t operator()(void);
private:
    const problem_data_t& _data;
};

class CreateGreedy : public onion::Creator< path_t >
{
public:
    CreateGreedy(const problem_data_t& d):
        Creator< path_t >("CreateGreedy"),_data(d){}
    virtual path_t operator()(void);
private:
    const problem_data_t& _data;
};


//-----------------------------------------------------------------
//                  OBJECTIVE
//-----------------------------------------------------------------
using Objective = onion::Objective<path_t,problem_data_t>;

class tspObjective : public Objective{
public:

    tspObjective(const problem_data_t& d):
        Objective<path_t,problem_data_t>(&d,"tspObjective"){}
    virtual Objective::cost_type operator()(const path_t&);
};


//-----------------------------------------------------------------
//                  NEIGHBORHOOD
//-----------------------------------------------------------------

class _2opt : public onion::Neighbor<path_t>{
public:
    _2opt():Neighbor("_2opt"){}
    std::vector<path_t> operator()(const path_t& path);
private:
};

class RemoveReinsert : public onion::Neighbor<path_t>{
public:
    RemoveReinsert(unsigned length = 0)
        :Neighbor("RemoveReinsert"),_length(length){
    }
    std::vector<path_t> operator()(const path_t& path);
private:
    unsigned _length;
};


using Creator = Creator<path_t>;
using Neighbor = Neighbor<path_t>;
using CreatorCallsCounter = CreatorCallsCounter<path_t>;
using ObjectiveCallsCounter = ObjectiveCallsCounter<path_t, problem_data_t>;
using ObjectiveRecorder = ObjectiveRecorder<path_t,problem_data_t>;

using Updater = min::Updater<path_t>;
using UpdateRecorder = min::UpdateRecorder<path_t>;
using UpdateLocalRecorder = min::UpdateLocalRecorder<path_t>;
using UpdateStagnationCounter = min::UpdateStagnationCounter<path_t>;
using UpdateImprovementMeter = min::UpdateImprovementMeter<path_t>;
using UpdateResetObject = min::UpdateResetObject<path_t>;

std::ostream& operator<<(std::ostream &os, const path_t &path);

//-----------------------------------------------------------------
//                      DELTA VERSIONS
//-----------------------------------------------------------------

//namespace delta{
//struct mask_tranf_t{
//    unsigned _mask_start;
//    unsigned _mask_length;
//    std::vector<unsigned> _positions;
//};

//class MaskReinsert : public onion::delta::Neighbor<path_t,mask_tranf_t>{
//public:
//    MaskReinsert(unsigned length = 0):_length(length){}
//    mask_tranf_t operator()(const path_t& path);
//private:
//    unsigned _length;
//};

//using Objective =
//onion::delta::Objective<
//path_t, mask_tranf_t, tsp_problem_data_t, std::vector<int> >;

//class tspObjective : public Objective{
//public:

//    tspObjective(const tsp_problem_data_t& d):Objective(d){}

//    virtual Objective::cost_type operator()(const path_t&, const mask_tranf_t&);
//};


//using Neighbor  = onion::delta::Neighbor<path_t,mask_tranf_t>;
//using Updater   = onion::delta::min::Updater<path_t>;

//}

} // namespace path

//-----------------------------------------------------------------
//                      BITMATRIX IMPL
//-----------------------------------------------------------------

//namespace bitmatrix{

//class CreateRandom : public onion::Creator< bitmatrix_t >
//{
//public:
//    CreateRandom(size_t sz):_size(sz){}
//    virtual bitmatrix_t operator()(void);
//private:
//    size_t _size;
//};

//using ObjectiveBase = onion::Objective<bitmatrix_t,tsp_problem_data_t>;

//class Objective : public ObjectiveBase{
//public:

//    Objective(const tsp_problem_data_t& d);
//    using cost_t = ObjectiveBase::cost_type;
//    virtual cost_t get(const bitmatrix_t&);
//};

//std::ostream& operator<<(std::ostream &os, const bitmatrix_t &path);

//}

// matrix <-> path conversions
//path_t to_path(const bitmatrix_t&);
//bitmatrix_t to_bitmatrix(const path_t&);

//-----------------------------------------------------------------
//                      ACCEPT (COMMON TO ALL IMPL.)
//-----------------------------------------------------------------

using Accept = min::Accept<unsigned>;
using Accept1st = min::Accept1st<unsigned>;
using AcceptBest = min::AcceptBest<unsigned>;
using AcceptStagCounter = min::AcceptStagCounter<unsigned>;

//namespace delta{

//using Accept        = onion::delta::min::Accept<>;
//using Accept1st     = onion::delta::min::Accept1st<>;
//using AcceptBest    = onion::delta::min::AcceptBest<>;
//using Updater       = onion::delta::min::Updater<tsp::path_t>;

//}

}}}// namespace onion::cops::tsp


#endif // ATSP_H
