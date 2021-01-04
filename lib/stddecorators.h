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
#include "accept.h"

namespace onion{

//-----------------------------------------------------------------
//                  LOOP CONTROLLER DECORATORS
//-----------------------------------------------------------------

class LoopTimer :
        public LoopController,
        public Timer,
        public OnionLayer<LoopController>
{
public:
    LoopTimer(OnionLayer<LoopController>::core_ptr_t next):
        LoopController(0,"Timer"),OnionLayer<LoopController>(next){}

    virtual bool operator()();
};

class LoopCounter :
        public LoopController,
        public Counter,
        public OnionLayer<LoopController>
{
public:
    LoopCounter(OnionLayer<LoopController>::core_ptr_t next):
        LoopController(0,"Counter"),OnionLayer<LoopController>(next){}
    virtual bool operator()();
};


class LoopRecorder :
        public LoopController,
        public Recorder,
        public OnionLayer<LoopController>
{
public:
    LoopRecorder(OnionLayer<LoopController>::core_ptr_t next, unsigned regularity = 1):
        LoopController(0,"Recorder"),Recorder(regularity),OnionLayer<LoopController>(next){}
    virtual bool operator()();
};

class LoopResetObject :
        public LoopController,
        public OnionLayer<LoopController>
{
public:
    LoopResetObject(OnionLayer<LoopController>::core_ptr_t next):
        LoopController(0,"Resetter"),OnionLayer<LoopController>(next){}
    virtual bool operator()();

    void setObject( std::shared_ptr<AResettable> ptr){
        _object = ptr;
    }

private:

    std::shared_ptr<AResettable> _object;
};

//-----------------------------------------------------------------
//                  CREATE DECORATORS
//-----------------------------------------------------------------

template<typename solution_t>
class CreatorCallsCounter:
        public Creator<solution_t>,
        public Counter,
        public OnionLayer<Creator<solution_t>>
{
public:

    CreatorCallsCounter( typename OnionLayer<Creator<solution_t>>::core_ptr_t next):
        Creator<solution_t>("Calls Counter"),OnionLayer<Creator<solution_t>>(next){}

    virtual solution_t operator()(){
        this->count();
        return this->_next->operator()();
    }

};

//-----------------------------------------------------------------
//                  UPDATE DECORATORS
//-----------------------------------------------------------------

template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c>
class UpdateStagnationCounter :
        public Updater<solution_t,cost_t,c>,
        public Counter,
        public OnionLayer<Updater<solution_t,cost_t,c>>
{
public:
    using OnionLayerBase = OnionLayer<Updater<solution_t,cost_t,c>>;
    using Updater = Updater<solution_t,cost_t, c>;

    UpdateStagnationCounter(typename OnionLayerBase::core_ptr_t next):
        Updater("Stagnation Counter"),Counter(0),OnionLayerBase(next){}

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
    using Updater = Updater<solution_t,cost_t, c>;

    UpdateRecorder(typename OnionLayerBase::core_ptr_t next, unsigned regularity = 1):
        Updater("Recorder"),Recorder(regularity),OnionLayerBase(next){}

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
    using Updater = Updater<solution_t,cost_t, c>;

    UpdateLocalRecorder(typename OnionLayerBase::core_ptr_t next, unsigned regularity = 1):
        Updater("Local Recorder"),Recorder(regularity),OnionLayerBase(next),_candidate_cost(0),
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

template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c>
class UpdateImprovementMeter :
        public Updater<solution_t,cost_t, c>,
        public ResettableValue<double>,
        public OnionLayer<Updater<solution_t,cost_t,c>>
{
public:
    using OnionLayerBase = OnionLayer<Updater<solution_t,cost_t,c>>;
    using Updater = Updater<solution_t,cost_t, c>;

    UpdateImprovementMeter(typename OnionLayerBase::core_ptr_t next):
        Updater("Improvemet Meter"),OnionLayerBase(next){
    }

    virtual bool operator()(solution_t& bestSoFar,
                        cost_t& bsfCost,
                        const solution_t& candidate,
                        const cost_t candidateCost )
    {
        if (this->getValue() == 0 ){
            startcost = bsfCost;
        }

        auto result = (*this->_next)(bestSoFar,bsfCost,candidate, candidateCost);
        const auto abs_startcost = std::abs( static_cast<long>(startcost) );
        if (result) this->setValue( static_cast<double>( std::abs( static_cast<long>(startcost - candidateCost) ) ) / abs_startcost );
        return result;
    }

private:

    cost_t startcost;

};

template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c>
class UpdateResetObject :
        public Updater<solution_t,cost_t,c>,
        public OnionLayer<Updater<solution_t,cost_t,c>>
{
public:
    using OnionLayerBase = OnionLayer<Updater<solution_t,cost_t,c>>;
    using Updater = Updater<solution_t,cost_t, c>;

    UpdateResetObject(typename OnionLayerBase::core_ptr_t next):
        Updater("Reset Object"),OnionLayerBase(next){}

    virtual bool operator()(solution_t& bestSoFar,
                        cost_t& bsfCost,
                        const solution_t& candidate,
                        const cost_t candidateCost )
    {
       auto result = (*this->_next)(bestSoFar,bsfCost,candidate, candidateCost);
       _object->reset();
       return result;
    }

    void setObject( std::shared_ptr<AResettable> ptr){
        _object = ptr;
    }

private:

    std::shared_ptr<AResettable> _object;
};


namespace max{

template< typename solution_t,typename cost_t = unsigned>
using UpdateStagnationCounter =
UpdateStagnationCounter<solution_t,cost_t,Compare<cost_t>::greater>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateImprovementMeter =
UpdateImprovementMeter<solution_t,cost_t,Compare<cost_t>::greater>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateRecorder =
UpdateRecorder<solution_t,cost_t,Compare<cost_t>::greater>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateLocalRecorder =
UpdateLocalRecorder<solution_t,cost_t,Compare<cost_t>::greater>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateResetObject =
UpdateResetObject<solution_t,cost_t,Compare<cost_t>::greater>;

}

namespace min{

template< typename solution_t,typename cost_t = unsigned>
using UpdateStagnationCounter =
UpdateStagnationCounter<solution_t,cost_t,Compare<cost_t>::less>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateImprovementMeter =
UpdateImprovementMeter<solution_t,cost_t,Compare<cost_t>::less>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateRecorder =
UpdateRecorder<solution_t,cost_t,Compare<cost_t>::less>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateLocalRecorder =
UpdateLocalRecorder<solution_t,cost_t,Compare<cost_t>::less>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateResetObject =
UpdateResetObject<solution_t,cost_t,Compare<cost_t>::less>;

}

//-----------------------------------------------------------------
//                  OBJECTIVE DECORATORS
//-----------------------------------------------------------------

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
    using Objective = Objective<solution_t,problem_data_t, cost_t>;
    ObjectiveCallsCounter(typename OnionLayer::core_ptr_t next):
        Objective("Calls Counter"),OnionLayer(next){}

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
    using Objective = Objective<solution_t,problem_data_t, cost_t>;

    ObjectiveRecorder(typename OnionLayer::core_ptr_t next, unsigned regularity = 1 ):
        Objective("Recorder"),Recorder(regularity),OnionLayer(next){}

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

//-----------------------------------------------------------------
//                  ACCEPT DECORATORS
//-----------------------------------------------------------------


template< typename cost_t,
          typename Compare<cost_t>::compare_fcn_t compare>
class AcceptStagCounter :
        public Accept<cost_t,compare>,
        public Counter,
        public OnionLayer<Accept<cost_t,compare>>
{
public:
    using OnionLayer = OnionLayer<Accept<cost_t,compare>>;
    using Accept = Accept<cost_t,compare>;

    AcceptStagCounter(typename OnionLayer::core_ptr_t next):
        Accept("Stagnation Counter"),OnionLayer(next){}

    virtual AcceptResult operator()(const cost_t&c ,const std::vector<cost_t>& v){
       auto result = (*this->_next)(c,v);
       if (!result)
           this->count();
       else
           this->reset();
       return result;
    }

};

namespace max{

template< typename cost_t>
using AcceptStagCounter = AcceptStagCounter<cost_t,Compare<cost_t>::greater>;

}
namespace min{

template< typename cost_t>
using AcceptStagCounter = AcceptStagCounter<cost_t,Compare<cost_t>::less>;

}


} // namespace onion
#endif // STDDECORATORS_H
