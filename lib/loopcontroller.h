#ifndef LOOPCONTROLLER_H
#define LOOPCONTROLLER_H

#include <string>
#include <vector>

#include "facilities.h"
#include "abstractvalues.h"

namespace onion{

class __Trigger : public onion::LabeledObject
{
public:
    explicit __Trigger(const std::string& label);
    virtual ~__Trigger() = default;
    virtual bool activated() const = 0;
    virtual void reset() = 0;
};


template <typename T = unsigned>
class Trigger : public __Trigger
{
public:

    Trigger(std::string label, AResettableValue<T>& v, T limit, bool external = false, Compare<T> c = Compare<T>::greater_or_equal ):
        __Trigger(label),_value(v),_limit(limit),_compare(c),_external(external){}
    Trigger(std::string label, AResettableValue<T>&& v, T limit, bool external = false, Compare<T> c = Compare<T>::greater_or_equal ):
        __Trigger(label),_value(v),_limit(limit),_compare(c),_external(external){}

    virtual bool activated() const{
        return _compare( _value.getValue(), _limit );
    }
    virtual void reset(){
        if (!_external) _value.reset();
    }


private:

    AResettableValue<T>& _value;
    const T _limit;
    Compare<T> _compare;
    bool _external;

};



class LoopController : public NonCopyable
{
public:

    LoopController() = default;
    virtual ~LoopController() = default;
    virtual bool operator()();

    template<typename T> void addTrigger( const Trigger<T>& t){
        _triggers.push_back( std::make_shared<Trigger<T>>(t) );
    }

    void reset();

    std::string getTrigger() const;

private:
    using trigger_ptr_t = std::shared_ptr<__Trigger>;
    std::vector<trigger_ptr_t> _triggers;
    std::string _triggerID;
};


}

#endif // EXECUTION_CONTROLLER_H
