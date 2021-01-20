#include "mkp.h"
#include "../lib/random.h"
#include <iostream>
#ifdef __DEBUG__
#include <cassert>
#endif

using namespace std;
using namespace onion::cops::mkp;

mkp_data_t mkp_datDataLoader::operator()(std::istream& is)
{
    unsigned products = 0;
    unsigned dimensions = 0;
    unsigned optimal_value = 0;

    is >> products >> dimensions >> optimal_value;

    mkp_data_t data(dimensions, products );

    for(unsigned i = 0; i < products; ++i)
      is >> data._profits.at(i);

    for(unsigned i = 0; i < dimensions; ++i)
      for(unsigned j = 0; j < products; ++j)
          is >> data._costs.at(i).at(j);

    for(unsigned i = 0; i < dimensions; ++i)
      is >> data._capacities.at(i);

    return data;
}

namespace  {

#ifdef __DEBUG__
bool isValidSolution( const mkp_sol_t& s, const mkp_data_t& _data ){
    const auto products = _data._products;
    const auto dimensions = _data._dimensions;
    std::vector<unsigned> sum(dimensions,0);

    for(unsigned dim = 0; dim < dimensions; dim++){
        for(unsigned prod = 0; prod < products; prod++ )
            if ( s.at(prod) )
                sum.at(dim) += _data._costs.at(dim).at(prod);

            if (sum.at(dim) > _data._capacities.at(dim) )
                return false;
    }
    return true;
}
#endif

struct ui{ // utility-index
    unsigned index   = 0;
    double   utility = 0;
};

bool ui_greater(const ui& a, const ui& b){
    return (a.utility > b.utility);
}
bool ui_less(const ui& a, const ui& b){
    return (a.utility < b.utility);
}

using utility_fcn_ptr = double(*)(const mkp_data_t& d, unsigned& prod_index );
using compare_fcn_ptr = bool (*)(const ui& a, const ui& b);
using prod_rank_fcn_ptr =
std::vector<size_t> (*)(const mkp_data_t&, compare_fcn_ptr);

std::vector<size_t> OrderByFcn(const mkp_data_t& data,
                                  utility_fcn_ptr util_fcn,
                                  compare_fcn_ptr cf = ui_greater){

    const auto products = data._products;
    std::vector<ui> uivec(products);

    for(unsigned prod = 0; prod < products; prod++){
        uivec.at(prod).index    = prod;
        uivec.at(prod).utility  = util_fcn(data,prod);
    }

    std::sort(uivec.begin(),uivec.end(), cf);

    std::vector<size_t> order(products,0);

    for(unsigned i = 0; i < products; i++)
        order.at(i) = uivec.at(i).index;

    return std::vector<size_t>(order);
}

std::vector<size_t> OrderByProfit(const mkp_data_t& data,
                                     compare_fcn_ptr cfptr = ui_greater){
    return OrderByFcn(data, []( const mkp_data_t& d, unsigned& prod )
    {
        return static_cast<double>(d._profits[prod]);
    }, cfptr );
}

std::vector<size_t> OrderIota(const mkp_data_t& data,
                              compare_fcn_ptr cfptr = ui_greater){

    static std::vector<size_t> order(data._products,0);

    std::iota(order.begin(),order.end(),0);
    if ( cfptr == ui_less ) std::reverse(order.begin(),order.end());

    return order;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
std::vector<size_t> OrderRandom(const mkp_data_t& data,
                              compare_fcn_ptr cfptr = nullptr){

    static std::vector<size_t> order(data._products,0);

    std::iota(order.begin(),order.end(),0);
    std::shuffle(order.begin(),order.end(),onion::get_random_engine());

    return order;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

std::vector<size_t> OrderByP2TWR(
        const mkp_data_t& data,
        compare_fcn_ptr cfptr = ui_greater ){

    return OrderByFcn(data, []( const mkp_data_t& data, unsigned& prod )
    {
        double total_weight = 0.0;
        for(unsigned dim = 0; dim < data._dimensions; dim++)
            total_weight += data._costs[dim][prod];
        return static_cast<double>( data._profits[prod] ) / total_weight;
    }, cfptr );
}

// Weight Index To Capacity Ratio

std::vector<size_t> OrderByP2WIR(
        const mkp_data_t& data,
        compare_fcn_ptr cfptr = ui_greater ){

    return OrderByFcn(data, []( const mkp_data_t& data, unsigned& prod )
    {
        double weight_index = 0.0;
        for(unsigned dim = 0; dim < data._dimensions; dim++)
            weight_index += static_cast<double>(data._costs[dim][prod] )
                    / data._capacities[dim];
        return static_cast<double>( data._profits[prod] ) / weight_index;
    }, cfptr );
}

mkp_sol_t Create(const std::vector<size_t>& prod_order,
                 const mkp_data_t& data){

    const auto              products            = data._products;
    const auto              dimensions          = data._dimensions;
    std::vector<unsigned>   sum(dimensions,0);
    mkp_sol_t               sol(products,false);

    for(unsigned i = 0; i < products; i++){
        bool overload = false;

        auto prod = prod_order.at(i);

        for(unsigned dim = 0; dim < dimensions; dim++ ){
           sum.at(dim) += data._costs.at(dim).at(prod);
           if ( sum.at(dim) > data._capacities.at(dim) )
               overload = true;
        }

        if (overload)
            for(unsigned dim = 0; dim < dimensions; dim++ )
                sum.at(dim) -= data._costs.at(dim).at(prod);
        else
            sol.at(prod) = true;
    }
#ifdef __DEBUG__

    auto sum2 = std::vector<unsigned>( dimensions,0);
    for(unsigned prod = 0; prod < products; prod++){
        if ( sol.at(prod) ){
            for(unsigned dim = 0; dim < dimensions; dim++ ){
                sum2.at(dim) += data._costs.at(dim).at(prod);
            }
        }
    }
    assert( sum2 == sum );

#endif

    return mkp_sol_t(sol);

}

} // end of anonymous namespace

mkp_sol_t CreateByItemOrder::operator()()
{
    static const std::vector<size_t> order = OrderIota(_data);
    return Create(order,_data);
}

mkp_sol_t CreateByItemProfit::operator()()
{
    static const std::vector<size_t> order = OrderByProfit(_data);
    return Create(order,_data);
}

mkp_sol_t CreateRandom::operator()()
{
    static std::vector<size_t> order(_data._products,0);
    order = OrderRandom(_data);
    return Create(order,_data);
}
mkp_sol_t CreateByItemUtilityTW2CR::operator()()
{
    static const std::vector<size_t> order = OrderByP2TWR(_data);
    return Create(order,_data);
}

mkp_sol_t CreateByItemUtilityWI2CR::operator()()
{
    static const std::vector<size_t> order = OrderByP2WIR(_data);
    return Create(order,_data);
}

Objective::cost_type mkpObjective::operator()(const mkp_sol_t & s)
{
    const auto& products = _data->_products;
    Objective::cost_type profit = 0;
    for(unsigned prod =0; prod < products; prod++ ){
        if ( s.at(prod) ) profit += _data->_profits.at(prod);
    }
    return profit;
}

const std::vector<unsigned>& getResourceUsage(const mkp_data_t &d, const mkp_sol_t& s){
    const auto products = d._products;
    const auto dimensions = d._dimensions;
    static std::vector<unsigned> sum(dimensions);

    for(unsigned dim = 0; dim < dimensions; dim++){
        sum.at(dim) = 0;
        for(unsigned prod = 0; prod < products; prod++ ){
            if ( s.at(prod) )
                sum.at(dim) += d._costs.at(dim).at(prod);
        }
    }
    return sum;
}

static prod_rank_fcn_ptr OrderStrategies [] = {
    OrderIota,
    OrderByProfit,
    OrderRandom,
    OrderByP2TWR,
    OrderByP2WIR
};

std::vector<mkp_sol_t>
InsertRepair::operator()(const mkp_sol_t &s)
{
    static const auto&                  products            = _data._products;
    static const auto&                  dimensions          = _data._dimensions;
    static const auto&                  rev_utility_index   = OrderStrategies[_prstrat](_data, ui_less );
    static const auto&                  utility_index       = OrderStrategies[_prstrat](_data, ui_greater);
    static const unsigned               maxp                = products-1;
    static std::vector<int>             delta_r(dimensions);
    static std::vector<mkp_sol_t>       newsol(1);

    unsigned newProdIndex = onion::rand_between(0,maxp);
    while( s.at(newProdIndex) )
       newProdIndex = onion::rand_between(0,maxp);

    const auto& res_usage = getResourceUsage(_data,s);

    for(unsigned i = 0; i < dimensions; ++i)
        delta_r[i] = static_cast<int>(_data._capacities[i] - res_usage[i] - _data._costs[i][newProdIndex]);

    newsol[0] = s;
    newsol[0][newProdIndex] = true;

#ifdef __DEBUG__
    assert( isValidSolution(s,_data) );
    assert( !isValidSolution(newsol[0],_data) );
#endif

    // removes products until solution valid, least utility 1st
    for(const auto& i : rev_utility_index){
        if (i == newProdIndex) continue;
        if ( newsol[0][i] ) {
            bool fixed = true;
            newsol[0][i] = false;
            for(unsigned d = 0; d < dimensions; ++d){
                delta_r[d] += static_cast<int>( _data._costs[d][i] );
                if (delta_r[d] < 0 ) fixed = false;
            }
            if ( fixed ) break;
        }
    }

#ifdef __DEBUG__
    assert( isValidSolution(newsol[0],_data) );
#endif

    // insert products untill solution valid, greater utility 1st
    for(const auto& i : utility_index){
        if ( newsol[0][i] ) continue;
        bool viable_prod = true;
        for(unsigned d = 0; d < dimensions; ++d)
            if ( delta_r[d] - static_cast<int>( _data._costs[d][i] ) < 0 ){
                viable_prod = false;
                break;
            }
        if (viable_prod){
           newsol[0][i] = true;
           for(unsigned d = 0; d < dimensions; ++d)
               delta_r[d] -= static_cast<int>( _data._costs[d][i] );
        }
    }

#ifdef __DEBUG__
    assert( isValidSolution(newsol[0],_data) );
#endif

    return newsol;
}

std::string onion::cops::mkp::ProductRankStrategyStr(ProductRankStrategy prs){
    static const char * const _rank_strategy_str[5] = {"Default", "ByProfit", "Random", "P2TWR", "P2WIR"};
    return _rank_strategy_str[static_cast<int>(prs)];
}
