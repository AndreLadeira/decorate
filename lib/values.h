#ifndef VALUES_H
#define VALUES_H

#include <memory>
#include <ctime>

namespace onion {


template<typename T>
struct AValue{
    virtual ~AValue() = default;
    virtual T getValue() const = 0;
    virtual void setValue(const T& v) = 0;
};


struct AResettable
{
    AResettable(bool locked = false);
    virtual ~AResettable();
    virtual void reset(){
        if (!_locked) this->doReset();
    }
    virtual void hardReset(){
        this->doReset();
    }

protected:

    virtual void doReset() = 0;

private:

    bool _locked;
};

template<typename T>
struct Value:
        public AValue<T>{

    explicit Value(T v0 = T(0)):_v(v0){}
    virtual ~Value() = default;
    virtual T getValue() const { return _v;}
    virtual void setValue(const T& v) {_v = v;}

protected:

    T _v;

};

template<typename T>
struct ResettableValue :
        public Value<T>,
        public AResettable
{
    explicit ResettableValue(T v = T(0), bool locked = false):
        Value<T>(v),AResettable(locked),_v0(v){}

    virtual ~ResettableValue() = default;

    virtual void doReset(){
           Value<T>::_v = _v0;
    }

private:

    T _v0;
};

template<typename T>
struct RefValue:
        public AValue<T>
{
    explicit RefValue(T& v):_vref(v){}
    //explicit RefValue(T&& v):_vref(v){}
    virtual ~RefValue() = default;
    virtual T getValue(void) const {
        return _vref;
    }
    virtual void setValue(const T& v){
        _vref = v;
    }

protected:

    T& _vref;
};

struct ACounter{
    virtual void count(unsigned amount) = 0;
    virtual ~ACounter();
};

struct Counter :
        public Value<unsigned>,
        public ACounter
{
    explicit Counter(unsigned start = 0);
    virtual ~Counter() = default;
    void count(unsigned amount = 1);

};

struct ResettableCounter :
        public ResettableValue<unsigned>,
        public ACounter{

    explicit ResettableCounter(unsigned start = 0, bool locked = false):
        ResettableValue<unsigned>(start,locked),_accumulated(0){}
    virtual ~ResettableCounter() = default;
    void count(unsigned amount = 1);
    unsigned getAccumulated();
private:
    unsigned _accumulated;
};

}


#endif // VALUES_H
