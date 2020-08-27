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


}


#endif // FACILITY_H
