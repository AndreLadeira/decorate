#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <string>
#include <map>
#include <istream>
#include <sstream>
#include "facilities.h"

namespace onion
{

class Parameter
{
public:

    explicit Parameter(std::string s = ""):_value(s){}

    template<typename T> T as(void) const {
        std::stringstream ss(_value);
        T retval;
        ss >> retval;
        return retval;
    }
    std::string str() const;
    std::string as() const;
    operator std::string() const;

private:

     std::string _value;
};


class ParameterList
{
public:

    ParameterList() = default;
    Parameter operator()(std::string _key) const;

protected:

    friend class ParameterLoader;

    void set(const std::string& _key, const std::string& _value);
    void clear();

private:

    std::map<std::string,Parameter> _list;
};

class ParameterLoader : public NonCopyable
{
public:
    ParameterLoader() = default;
    virtual ~ParameterLoader() = default;

    virtual void operator()(std::istream& is, ParameterList& paramList);
};

}
#endif // PARAMETERS_H
