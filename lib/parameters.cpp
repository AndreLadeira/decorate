#include "parameters.h"
#include <regex>

using namespace onion;
using namespace std;


string Parameter::str() const{
    return _value;
}

string Parameter::as() const {
    return str();
}

onion::Parameter::operator std::string() const{
    return str();
}

Parameter ParameterList::getValue(string _key) const
{
    if ( _list.find(_key) != _list.end())
        return _list.at(_key);
    else
        return Parameter();
}

void ParameterList::set(const string& _key, const string& _value){
    _list[_key] = Parameter(_value);
}

void ParameterList::clear(){
    _list.clear();
}

void ParameterLoader::operator()(istream& is, ParameterList& paramList)
{
    paramList.clear();
    while(!is.eof())
    {
        std::string param;
        is >> param;

        std::smatch match;
        std::regex exp("([^=\\s\\t]+)=([^\\s\\t]+)");

        if ( regex_search(param,match,exp) )
            paramList.set(match[1].str(), match[2].str());
    }
    if ( paramList.getValue("file_name").str() == "" )
        throw runtime_error("ParameterLoader: mandatory <file_name> parameter missing");
}
