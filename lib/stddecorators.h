#ifndef STDDECORATORS_H
#define STDDECORATORS_H

#include "facilities.h"
#include "decorator.h"
#include "loopcontroller.h"
#include "create.h"
#include "update.h"

namespace onion{



//template<typename T>
//class CallsCounter :
//        public T,
//        public Counter,
//        public Decorator<T>
//{
//    using DecoratorBase = Decorator<T>;
//    friend class Decorator<T>;

//    CallsCounter(typename DecoratorBase::ptr_t ptr):
//        Decorator<T>(ptr){}

//};

class LoopCallsCounter :
        public LoopController,
        public Counter,
        public Decorator<LoopController>
{
    using DecoratorBase = Decorator<LoopController>;
    friend class Decorator<LoopController>;

    LoopCallsCounter(typename DecoratorBase::ptr_t ptr):
        Decorator<LoopController>(ptr){}
    virtual bool running();
};

template<typename solution_t>
class CreatorCallsCounter:
        public Creator<solution_t>,
        public Counter,
        public Decorator<Creator<solution_t>>
{
    using DecoratorBase = Decorator<Creator<solution_t>>;
    friend class Decorator<Creator<solution_t>>;

    CreatorCallsCounter(typename DecoratorBase::ptr_t ptr):
        DecoratorBase(ptr){}
    virtual solution_t create(){
        this->count();
        return DecoratorBase::_ptr->create();
    }

};
template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c>
class UpdateStagnationCounter :
        public Updater<solution_t,cost_t, c>,
        public Counter,
        public Decorator<Updater<solution_t,cost_t,c>>
{
    using DecoratorBase = Decorator<Updater<solution_t,cost_t,c>>;
    friend class Decorator<Updater<solution_t,cost_t,c>>;

    UpdateStagnationCounter(typename DecoratorBase::ptr_t ptr):
        DecoratorBase(ptr){}

    virtual bool update(solution_t& bestSoFar,
                        cost_t& bsfCost,
                        const solution_t& candidate,
                        const cost_t candidateCost ) {
       auto result = DecoratorBase::_ptr->update(bestSoFar,bsfCost,
                                                 candidate, candidateCost);
       if (!result)
           this->count();
       else
           this->reset();

       return result;
    }
};

template< typename solution_t,typename cost_t = unsigned>
using UpdateUpStagCounter =
UpdateStagnationCounter<solution_t,cost_t,Compare<cost_t>::greater>;

template< typename solution_t,typename cost_t = unsigned>
using UpdateDownStagCounter =
UpdateStagnationCounter<solution_t,cost_t,Compare<cost_t>::less>;


}
#endif // STDDECORATORS_H
