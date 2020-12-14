#ifndef TIMER_H
#define TIMER_H

#include "values.h"
#include <ctime> // clock_t

namespace onion{

struct Timer :
        public ResettableValue<double>
{
    explicit Timer();
    virtual ~Timer() = default;
    virtual double getValue(void) const;
    virtual void setValue(const double& v);
    bool started() const;
    void start();
    void stop();
    virtual void reset();

protected:

    bool    _started;
    clock_t _begin;
    clock_t _end;

};

}

#endif // TIMER_H
