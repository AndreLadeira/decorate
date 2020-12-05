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
    virtual ~AResettable() = default;
    virtual void reset() = 0;
};

template<typename T>
struct Value: public AValue<T>
{

    explicit Value(T v0 = T(0)):_v(v0){}
    virtual ~Value() = default;
    virtual T getValue() const { return _v;}
    virtual void setValue(const T& v) {_v = v;}

    Value<T>& operator++(int){ _v++; return *this;}
    Value<T>& operator--(int){ _v--; return *this;}
    Value<T>& operator++(){ _v++; return *this;}
    Value<T>& operator--(){ _v--; return *this;}

protected:

    T _v;

};

template<typename T>
struct ResettableValue :
        virtual public Value<T>,
        public AResettable
{
    explicit ResettableValue(T v = T(0)):
        Value<T>(v),_v0(v){}

    virtual ~ResettableValue() = default;

    virtual void reset(){
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
    virtual ~ACounter() = default;
};

struct Counter :
        virtual public Value<unsigned>,
        public ACounter
{
    explicit Counter(unsigned start = 0);
    virtual ~Counter() = default;
    void count(unsigned amount = 1);

};

struct ResettableCounter :
        virtual public ResettableValue<unsigned>,
        virtual public Counter{

    explicit ResettableCounter(unsigned start = 0);
    virtual ~ResettableCounter() = default;
};

} // namespace onion


#endif // VALUES_H
