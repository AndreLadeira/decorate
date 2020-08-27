#ifndef TIMER_H
#define TIMER_H

#include "abstractvalues.h"
#include <ctime> // clock_t

namespace onion{

struct Timer :
        virtual public AValue<double>
{
    explicit Timer(bool startnow = false);
    virtual ~Timer() = default;
    virtual double getValue(void) const;
    void start();
    void stop();
protected:

    clock_t _begin;
    clock_t _end;
};

struct ResettableTimer :
        virtual public Timer,
        virtual public AResettableValue<double>
{
    ResettableTimer(bool startnow = false):
        Timer(startnow),AResettable(false){}
    virtual ~ResettableTimer() = default;

protected:
    virtual void doReset();
};


}

#endif // TIMER_H
