#ifndef LOOPCONTROLLER_H
#define LOOPCONTROLLER_H

#include <string>
#include <vector>
#include <sstream>

#include "facilities.h"
#include "values.h"
#include "observer.h"

namespace onion{

class __StopCondition : public onion::LabeledObject
{
public:
    explicit __StopCondition(const std::string& tag);
    virtual ~__StopCondition() = default;
    virtual bool satisfied() const = 0;
    virtual void reset() = 0;
    virtual std::string getLimit() const = 0;
};


template <typename T = unsigned>
class StopCondition : public __StopCondition
{
public:

    StopCondition(std::string label, ResettableValue<T>& v, T limit, Compare<T> c = Compare<T>::greater_or_equal ):
        __StopCondition(label),_value(v),_limit(limit),_compare(c){}
    StopCondition(std::string label, ResettableValue<T>&& v, T limit, Compare<T> c = Compare<T>::greater_or_equal ):
        __StopCondition(label),_value(v),_limit(limit),_compare(c){}

    virtual bool satisfied() const{
        return _compare( _value.getValue(), _limit );
    }
    virtual void reset(){
        _value.reset();
    }

    std::string getLimit() const{
        std::stringstream ss;
        //ss << std::fixed << std::setprecision(4);
        if ( _limit >= 1000 ) ss << std::scientific;
        ss << _limit;
        return ss.str();
    }

private:

    ResettableValue<T>& _value;
    const T             _limit;
    Compare<T>          _compare;
};

class LoopController :
        public NonCopyable,
        public Subject,
        public LabeledObject
{
public:

    LoopController(unsigned maxLoops = 0, const char * const& = "LoopController");
    virtual ~LoopController() = default;
    virtual bool operator()();

    enum class ON_STOP { RESET, NO_ACTION };

    template<typename T> void addStopCondition(
            const StopCondition<T>& t, ON_STOP action = ON_STOP::NO_ACTION ){
        _stopConditions.push_back( {action,std::make_shared<StopCondition<T>>(t)} );
    }

    void resetObject(AResettable& v){
        _resObjects.push_back(v);
    }

    void reset();

    std::string getStopCondition() const;
    std::string getConditions() const;

    unsigned getLoopCount(){
        return _loopCount.getValue();
    }



private:
    using stop_condition_ptr_t = std::shared_ptr<__StopCondition>;

    struct stop_condition_pair_t{
        ON_STOP                 _stop_action;
        stop_condition_ptr_t    _ptr;
    };

    std::vector<stop_condition_pair_t>  _stopConditions;
    std::string                         _stopConditionLabel;
    ResettableValue<unsigned>           _loopCount;
    std::vector< std::reference_wrapper<AResettable> > _resObjects;
};


}

#endif // EXECUTION_CONTROLLER_H
