#ifndef FACILITY_H
#define FACILITY_H

#include <limits>
#include <ctime>
#include <string>

namespace onion {

class NonCopyable
{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable( const NonCopyable& ) = delete;
    NonCopyable& operator=( const NonCopyable& ) = delete;
};

class LabeledObject
{
public:
    explicit LabeledObject(const std::string& l):_label(l){}
    std::string getLabel() const {return _label;}
private:
    std::string _label;
};


template<typename T>
struct Value
{
    explicit Value( bool resetable = true):_v(T(0)),_resetable(resetable){}
    Value(T v):_v(v){}
    virtual ~Value() = default;
    virtual T getValue(void) const{
        return _v;
    }
    virtual void reset(){
        if (_resetable) hardReset();
    }
    virtual void hardReset(){
        _v = T(0);
    }

protected:
    T _v;
private:
    bool _resetable;
};

struct Counter : public Value<unsigned>
{
    explicit Counter( bool resetable = true):Value<unsigned>(resetable){}
    Counter(unsigned v):Value<unsigned>(v){}

protected:

    virtual void count(unsigned amount = 1);
};

template<typename T>
struct Compare
{
    using compare_fcn_t = bool (*)(T,T);
    static bool less(const T a, const T b){return a<b;}
    static bool less_or_equal(const T a, const T b){return a<=b;}
    static bool greater(const T a, const T b){return a>b;}
    static bool greater_or_equal(const T a, const T b){return a>=b;}
    static bool equal_to(const T a, const T b){return a==b;}
    static bool _true(const T, const T){return true;}

    Compare(compare_fcn_t c = less):_compare(c){}
    bool operator()(const T a, const T b) const { return _compare(a,b); }

private:

    compare_fcn_t _compare;

};

struct Timer : public Value<double>
{
    Timer(bool startnow = false, bool resetable = true);
    void start();
    double getValue(void) const;
    void reset();
    void hardReset();
private:
    clock_t begin;
};



}


#endif // FACILITY_H
