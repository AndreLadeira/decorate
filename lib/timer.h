#ifndef TIMER_H
#define TIMER_H

#include "values.h"
#include <ctime> // clock_t

namespace onion{

struct Timer :
        public AValue<double>
{
    explicit Timer(bool startnow = false);
    virtual ~Timer() = default;
    virtual double getValue(void) const;
    virtual void setValue(const double& v);
    void start();

protected:

    clock_t begin;
};

struct ResettableTimer :
        public Timer,
        public AResettable
{
    ResettableTimer(bool startnow = false, bool locked = false):
        Timer(startnow),AResettable(locked){}
    virtual ~ResettableTimer() = default;

protected:

    virtual void doReset();
};


}

#endif // TIMER_H
