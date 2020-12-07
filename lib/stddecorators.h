#ifndef STDDECORATORS_H
#define STDDECORATORS_H

#include "facilities.h"
#include "onion.h"
#include "loopcontroller.h"
#include "create.h"
#include "update.h"
#include "objective.h"
#include "recorder.h"
#include "timer.h"

namespace onion{

class LoopTimer :
        public LoopController,
        public Timer,
        public OnionLayer<LoopController>
{
public:
    LoopTimer(OnionLayer<LoopController>::core_ptr_t next);
    virtual bool operator()();
};

class LoopCounter :
        public LoopController,
        public Counter,
        public OnionLayer<LoopController>
{
public:
    LoopCounter(OnionLayer<LoopController>::core_ptr_t next):
        OnionLayer<LoopController>(next){}
    virtual bool operator()();
};


class LoopRecorder :
        public LoopController,
        public Recorder,
        public OnionLayer<LoopController>
{
public:
    LoopRecorder(OnionLayer<LoopController>::core_ptr_t next, unsigned regularity = 1):
        Recorder(regularity),OnionLayer<LoopController>(next){}
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

    virtual solution_t operator()(){
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
        Counter(0),OnionLayerBase(next){}

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

template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c>
class UpdateRecorder :
        public Updater<solution_t,cost_t, c>,
        public Recorder,
        public OnionLayer<Updater<solution_t,cost_t,c>>
{
public:
    using OnionLayerBase = OnionLayer<Updater<solution_t,cost_t,c>>;

    UpdateRecorder(typename OnionLayerBase::core_ptr_t next, unsigned regularity = 1):
        Recorder(regularity),OnionLayerBase(next){}

    virtual bool operator()(solution_t& bestSoFar,
                        cost_t& bsfCost,
                        const solution_t& candidate,
                        const cost_t candidateCost )
    {
        auto result = (*this->_next)(bestSoFar,bsfCost,candidate, candidateCost);
        this->record();
        return result;
    }
};

template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c>
class UpdateLocalRecorder :
        public Updater<solution_t,cost_t, c>,
        public Recorder,
        public OnionLayer<Updater<solution_t,cost_t,c>>
{
public:
    using OnionLayerBase = OnionLayer<Updater<solution_t,cost_t,c>>;

    UpdateLocalRecorder(typename OnionLayerBase::core_ptr_t next, unsigned regularity = 1):
        Recorder(regularity),OnionLayerBase(next),_candidate_cost(0),
        _local_tr("local", _candidate_cost  )  {
        this->addTrack ( _local_tr );
    }

    virtual bool operator()(solution_t& bestSoFar,
                        cost_t& bsfCost,
                        const solution_t& candidate,
                        const cost_t candidateCost )
    {
        auto result = (*this->_next)(bestSoFar,bsfCost,candidate, candidateCost);
        _candidate_cost.setValue(candidateCost);
        this->record();
        return result;
    }

    template<class T>
    Track<T> getLocalTrack(){
        return this->template getTrack<T>("local");
    }

private:

    Value<cost_t> _candidate_cost;
    Track<cost_t> _local_tr;
};


namespace max{

template< typename solution_t,typename cost_t = unsigned>
using UpdateStagnationCounter =
UpdateStagnationCounter<solution_t,cost_t,Compare<cost_t>::greater>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateRecorder =
UpdateRecorder<solution_t,cost_t,Compare<cost_t>::greater>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateLocalRecorder =
UpdateLocalRecorder<solution_t,cost_t,Compare<cost_t>::greater>;

}

namespace min{

template< typename solution_t,typename cost_t = unsigned>
using UpdateStagnationCounter =
UpdateStagnationCounter<solution_t,cost_t,Compare<cost_t>::less>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateRecorder =
UpdateRecorder<solution_t,cost_t,Compare<cost_t>::less>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateLocalRecorder =
UpdateLocalRecorder<solution_t,cost_t,Compare<cost_t>::less>;

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

    ObjectiveCallsCounter(typename OnionLayer::core_ptr_t next): OnionLayer(next){}

    virtual cost_t operator()(const solution_t& s) {
        this->count();
        return (*this->_next)(s);
    }

    virtual std::vector<cost_t> operator()(const std::vector<solution_t>& S) {
        this->count(S.size());
        return (*this->_next)(S);
    }
};

template< typename solution_t,
          typename problem_data_t,
          typename cost_t = unsigned >
class ObjectiveRecorder :
        public Objective<solution_t,problem_data_t, cost_t>,
        public Recorder,
        public OnionLayer<Objective<solution_t,problem_data_t, cost_t>>
{
public:
    using OnionLayer = OnionLayer<Objective<solution_t,problem_data_t, cost_t>>;

    ObjectiveRecorder(typename OnionLayer::core_ptr_t next, unsigned regularity = 1 ):
        Recorder(regularity),OnionLayer(next){}

    virtual cost_t operator()(const solution_t& s) {
        auto result = (*this->_next)(s);
        this->record();
        return cost_t(result);
    }

    virtual std::vector<cost_t> operator()(const std::vector<solution_t>& S) {
        std::vector<cost_t> result;
        result.reserve(S.size());
        for( const auto& s: S )
            result.push_back( (*this)(s) );
        return std::vector<cost_t>(result);
    }
};

} // namespace onion
#endif // STDDECORATORS_H
