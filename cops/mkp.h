#ifndef KNAPSACK_H
#define KNAPSACK_H

#include "../lib/dataload.h"
#include "../lib/create.h"
#include "../lib/types.h"
#include "../lib/objective.h"
#include "../lib/neighbor.h"
#include "../lib/accept.h"
#include "../lib/update.h"
#include "../lib/stddecorators.h"

namespace onion{
namespace cops{
namespace mkp{

using mkp_solution_t = onion::array_of<bool>;

struct mkp_data_t
{
    mkp_data_t(unsigned dimensions, unsigned products ):
        _costs(dimensions,products),_profits(products),_capacities(dimensions),
        _products(products),_dimensions(dimensions){}

    mkp_data_t(){}

    onion::matrix_of<unsigned> _costs;
    onion::array_of<unsigned>  _profits;
    onion::array_of<unsigned>  _capacities;

    unsigned _products = 0;
    unsigned _dimensions = 0;
};

class mkp_datDataLoader : public onion::DataLoader<mkp_data_t>
{
public:
    mkp_data_t operator()(std::istream& is);
};

using mkp_sol_t = onion::array_of<bool>;

//-----------------------------------------------------------------
//                  CREATE
//-----------------------------------------------------------------

class CreateByItemOrder : public onion::Creator< mkp_sol_t >
{
public:
    CreateByItemOrder(const mkp_data_t& data):
        Creator< mkp_sol_t >("CreateByItemOrder"),_data(data){}
    virtual mkp_sol_t operator()(void);
private:
    const mkp_data_t& _data;
};

class CreateByItemProfit : public onion::Creator< mkp_sol_t >
{
public:
    CreateByItemProfit(const mkp_data_t& data):
        Creator< mkp_sol_t >("CreateByItemProfit"),_data(data){}
    virtual mkp_sol_t operator()(void);
private:
    const mkp_data_t& _data;
};

// Total Weight To Capacity Ratio

class CreateByItemUtilityTW2CR : public onion::Creator< mkp_sol_t >
{
public:
    CreateByItemUtilityTW2CR(const mkp_data_t& data):
        Creator< mkp_sol_t >("CreateByItemUtilityTW2CR"),_data(data){}
    virtual mkp_sol_t operator()(void);
private:
    const mkp_data_t& _data;
};

// Weight Index To Capacity Ratio

class CreateByItemUtilityWI2CR : public onion::Creator< mkp_sol_t >
{
public:
    CreateByItemUtilityWI2CR(const mkp_data_t& data):
        Creator< mkp_sol_t >("CreateByItemUtilityWI2CR"),_data(data){}
    virtual mkp_sol_t operator()(void);
private:
    const mkp_data_t& _data;
};

class CreateRandom : public onion::Creator< mkp_sol_t >
{
public:
    CreateRandom(const mkp_data_t& data):
        Creator< mkp_sol_t >("CreateRandom"),_data(data){}
    virtual mkp_sol_t operator()(void);
private:
    const mkp_data_t& _data;
};

//-----------------------------------------------------------------
//                  OBJECTIVE
//-----------------------------------------------------------------
using Objective = onion::Objective<mkp_sol_t,mkp_data_t>;

class mkpObjective : public Objective{
public:

    mkpObjective(const mkp_data_t& d):
        onion::Objective<mkp_sol_t,mkp_data_t>(&d,"mkpObjective"){}
    virtual Objective::cost_type operator()(const mkp_sol_t&);
};

enum class ProductRankStrategy : size_t {
    DEFAULT,PROFIT,RANDOM, P2TWR, P2WIR
};

std::string ProductRankStrategyStr(ProductRankStrategy prs);

//-----------------------------------------------------------------
//                  NEIGHBORHOOD
//-----------------------------------------------------------------

class InsertRepair : public onion::Neighbor<mkp_sol_t>{
public:
    InsertRepair(const mkp_data_t& d, ProductRankStrategy prstrat = ProductRankStrategy::DEFAULT):
        Neighbor<mkp_sol_t>( std::string("InsertRepair-") + ProductRankStrategyStr(prstrat) ),
        _data(d),_prstrat( static_cast<size_t>(prstrat)){
    }
    std::vector<mkp_sol_t> operator()(const mkp_sol_t&);
private:
    const mkp_data_t& _data;
    const size_t _prstrat;
};

using Accept = max::Accept<unsigned>;
using Accept1st = max::Accept1st<unsigned>;
using AcceptBest = max::AcceptBest<unsigned>;
using AcceptStagCounter = max::AcceptStagCounter<unsigned>;
using CreatorCallsCounter = CreatorCallsCounter<mkp_sol_t>;
using ObjectiveCallsCounter = ObjectiveCallsCounter<mkp_sol_t, mkp_data_t>;
using ObjectiveRecorder = ObjectiveRecorder<mkp_sol_t, mkp_data_t>;

using Updater = max::Updater<mkp_sol_t>;
using UpdateRecorder = max::UpdateRecorder<mkp_sol_t>;
using UpdateLocalRecorder = max::UpdateLocalRecorder<mkp_sol_t>;
using UpdateStagnationCounter = max::UpdateStagnationCounter<mkp_sol_t>;
using UpdateImprovementMeter = max::UpdateImprovementMeter<mkp_sol_t>;
using UpdateResetObject = max::UpdateResetObject<mkp_sol_t>;

using Creator = Creator<mkp_sol_t>;
using Neighbor = Neighbor<mkp_sol_t>;

}
}
}

#endif // KNAPSACK_H
