#ifndef ABSTRACTVALUES_H
#define ABSTRACTVALUES_H

namespace onion {

template<typename T>
struct AValue{
    virtual ~AValue() = default;
    virtual T getValue() const = 0;
};

struct AResettable{

    AResettable(bool locked = false);

    virtual ~AResettable() = default;

    virtual void reset();

    virtual void hardReset();

protected:

    virtual void doReset() = 0;

private:

    bool _locked;
};

template<typename T>
struct AResettableValue:
        public virtual AValue<T>,
        public virtual AResettable
{
};

template<typename T>
struct Value : virtual public AValue<T>
{
    explicit Value(T v = T(0)):_v(v){}
    virtual ~Value() = default;
    virtual T getValue(void) const{
        return _v;
    }

protected:
    T _v;
};

template<typename T>
struct ResettableValue :
        virtual public Value<T>,
        virtual public AResettableValue<T>
{
    explicit ResettableValue(T v = T(0), bool locked = false):
        Value<T>(v),AResettable(locked){}
    virtual ~ResettableValue() = default;
    virtual void doReset(){
           this->_v = T(0);
        }
};


template<typename T>
struct BoundValue:
        virtual public AValue<T>
{
    explicit BoundValue(T& v):_vref(v){}
    virtual ~BoundValue() = default;
    virtual T getValue(void) const{
        return _vref;
    }
private:
    T& _vref;
};

struct Counter :
        virtual public Value<unsigned>
{
    explicit Counter(unsigned start = 0);
    virtual ~Counter() = default;
    void count(unsigned amount = 1);
};

struct ResettableCounter :
        virtual public Counter,
        virtual public ResettableValue<unsigned>
{
    explicit ResettableCounter(unsigned start = 0,bool locked = false);
    virtual ~ResettableCounter() = default;
};

}


#endif // ABSTRACTVALUES_H
