#ifndef STDDECORATORS_H
#define STDDECORATORS_H

#include "facilities.h"
#include "onion.h"
#include "loopcontroller.h"
#include "create.h"
#include "update.h"
#include "objective.h"

namespace onion{

class LoopCallsCounter :
        public LoopController,
        public Counter,
        public OnionLayer<LoopController>
{
public:
    LoopCallsCounter(OnionLayer<LoopController>::core_ptr_t next):
        OnionLayer<LoopController>(next){}
    virtual bool operator()();
};

template<typename solution_t>
class CreatorCallsCounter:
        public Creator<solution_t>,
        public Counter,
        public OnionLayer<Creator<solution_t>>
{
public:

    CreatorCallsCounter( typename OnionLayer<Creator<solution_t>>::core_ptr_t next):
        OnionLayer<Creator<solution_t>>(next){}

<<<<<<< HEAD
    CreatorCallsCounter(typename DecoratorBase::ptr_t ptr):
        DecoratorBase(ptr){}
    virtual solution_t create(){
=======
    virtual solution_t operator()(){
>>>>>>> functors
        this->count();
        return this->_next->operator()();
    }

};

template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c>
class UpdateStagnationCounter :
        public Updater<solution_t,cost_t, c>,
        public Counter,
        public OnionLayer<Updater<solution_t,cost_t,c>>
{
public:
    using OnionLayerBase = OnionLayer<Updater<solution_t,cost_t,c>>;

    UpdateStagnationCounter(typename OnionLayerBase::core_ptr_t next):
        OnionLayerBase(next){}

    virtual bool operator()(solution_t& bestSoFar,
                        cost_t& bsfCost,
                        const solution_t& candidate,
                        const cost_t candidateCost )
    {
       auto result = (*this->_next)(bestSoFar,bsfCost,candidate, candidateCost);
       if (!result)
           this->count();
       else
           this->reset();

       return result;
    }
};

namespace max{

template< typename solution_t,typename cost_t = unsigned>
using UpdateStagnationCounter =
UpdateStagnationCounter<solution_t,cost_t,Compare<cost_t>::greater>;

}

namespace min{

template< typename solution_t,typename cost_t = unsigned>
using UpdateStagnationCounter =
UpdateStagnationCounter<solution_t,cost_t,Compare<cost_t>::less>;

}

template< typename solution_t,
          typename problem_data_t,
          typename cost_t = unsigned >
class ObjectiveCallsCounter :
        public Objective<solution_t,problem_data_t, cost_t>,
        public Counter,
        public OnionLayer<Objective<solution_t,problem_data_t, cost_t>>
{
public:
    using OnionLayer = OnionLayer<Objective<solution_t,problem_data_t, cost_t>>;

    ObjectiveCallsCounter(typename OnionLayer::core_ptr_t next, bool resetable = true):
        Counter(resetable),OnionLayer(next){}

    virtual cost_t operator()(const solution_t& s) {
        this->count();
        return (*this->_next)(s);
    }

    virtual std::vector<cost_t> operator()(const std::vector<solution_t>& S) {
        this->count(S.size());
        return (*this->_next)(S);
    }
};

} // namespace onion
#endif // STDDECORATORS_H
