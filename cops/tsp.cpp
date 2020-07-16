#include "tsp.h"
#include <regex>
#include <algorithm>
#include <map>
#include <random>
#include <sstream>
#include "../lib/random.h"

using namespace std;
using namespace onion::cops::tsp;
using namespace onion::cops::tsp::path;

namespace  {

struct data_section_data{ string key; string value; };

data_section_data getDataSectionData(const string& str){
    smatch match;
    regex exp("^[\\s\\t]*(\\w*)[\\s\\t]*:?[\\s\\t]*([\\w]*)");

    if ( regex_search (str,match,exp) && match[1].str() != "COMMENT" )
        return { match[1].str(), match[2].str() };
    else
        return data_section_data();
}

static const vector<string> data_section_labels =
{
    "NODE_COORD_SECTION",
    "DEPOT_SECTION",
    "DEMAND_SECTION",
    "EDGE_DATA_SECTION",
    "FIXED_EDGES_SECTION",
    "DISPLAY_DATA_SECTION",
    "TOUR_SECTION",
    "EDGE_WEIGHT_SECTION",
};

inline bool is_data_section_key(const string& key)
{
    return find(data_section_labels.cbegin(),
                data_section_labels.cend(),
                key) != data_section_labels.end();
}

struct DataLoader
{
    virtual tsp_problem_data_t load(std::istream& is, unsigned sz) const = 0;
    virtual ~DataLoader() = default;
};

struct : public DataLoader
{
    tsp_problem_data_t load(std::istream& is, unsigned sz) const
    {
        tsp_problem_data_t data(sz,sz);

        for(unsigned i = 0; i < sz; i++)
           for(unsigned j = 0; j < sz; j++){
                is >> data[i][j];
           }

        return tsp_problem_data_t( data );
    }
} static tspExplicitLoader;


struct data_loader_id {
    string type;
    string edge_weight_type;
    string edge_weight_format;

    bool operator<(const data_loader_id& rhs) const{
        return (type + edge_weight_type + edge_weight_format) <
               ( rhs.type + rhs.edge_weight_type + rhs.edge_weight_format);
    }

    string to_string(){
        return type + "/" + edge_weight_type + "/" + edge_weight_format;
    }
};

static map<data_loader_id, const DataLoader& > DataLoaders = {
    { {"ATSP", "EXPLICIT", "FULL_MATRIX" }, tspExplicitLoader },
    { { "TSP", "EXPLICIT", "FULL_MATRIX" }, tspExplicitLoader }
};

}// end of anonymous namespace


tsp_problem_data_t tsp_tsplibDataLoader::operator()(istream& is)
try
{
    map<string,string>  data_section;
    while(!is.eof())
    {
        string s;
        data_section_data data;
        getline(is,s);
        data = ::getDataSectionData(s);

        if ( data.key == "" ) continue;

        if ( !is_data_section_key(data.key) )
            data_section.insert({data.key,data.value});
        else
            break;
    }

    const vector<string> req_data = { "TYPE", "DIMENSION", "EDGE_WEIGHT_TYPE" };
    for (auto& dat : req_data)
        if (data_section.find(dat) == data_section.end())
            throw runtime_error( "tsp_tsplibDataLoader: required " + dat + " information is missing.");

    unsigned sz = 0;
    try { sz = static_cast<unsigned>( stoul(data_section["DIMENSION"]) ); }
    catch(const out_of_range&){throw runtime_error("tsp_tsplibDataLoader: corrupted DIMENSION data");}

    data_loader_id type = {
        data_section["TYPE"],
        data_section["EDGE_WEIGHT_TYPE"],
        data_section["EDGE_WEIGHT_FORMAT"] };

    if ( DataLoaders.find(type) != DataLoaders.end() )
        return DataLoaders.at(type).load(is,sz);
    else
        throw runtime_error( "tsp_tsplibDataLoader: unsupported tsplib format: " + type.to_string() );
}
catch(istream::failure& ){
    throw runtime_error("tsp_tsplibDataLoader: corrupt data file");
}

path_t path::CreateRandom::create()
{
    path_t path(_size+1);
    // create a random path

    std::iota(path.begin(),path.end()-1,0);

    // begin+1 / end-1 to make paths always starting and ending at city 0,
    // with no loss of generality

    std::shuffle(path.begin()+1,path.end()-1,onion::get_random_engine());

    return path_t(path);
}

std::ostream& path::operator<<(std::ostream &os, const path_t &path)
{
    for(auto city : path){
        os<< city << " ";
    }
    os << "\n";
    return os;
}

bitmatrix_t bitmatrix::CreateRandom::operator()(){
    bitmatrix_t matrix(_size,_size);

    return bitmatrix_t( to_bitmatrix( path::CreateRandom(_size)() ));
}


path_t onion::cops::tsp::to_path(const bitmatrix_t & matrix)
{
    const auto& sz = matrix.size();
    path_t path(sz+1);
    unsigned current = 0;
    for(unsigned i = 1; i < sz; i++)
        for(unsigned j = 0; j < sz; j++)
            if ( matrix.at(current).at(j) )
            {
                path.at(i) = j;
                current = j;
                break;
            }
    return path_t(path);
}

bitmatrix_t onion::cops::tsp::to_bitmatrix(const path_t & path)
{
    const auto& sz = path.size();
    bitmatrix_t matrix(sz-1,sz-1);
    unsigned prev = 0;
    for(unsigned i = 1; i < sz; ++i )
    {
        unsigned next = path.at(i);
        matrix.at(prev).at(next) = true;
        prev = next;
    }
    return bitmatrix_t(matrix);
}

namespace{

path_t _2optSwap(const path_t& path, unsigned start, unsigned length)
{

#ifdef __DEBUG__
    auto sz = path.size();
    if ( start < 1 || start > sz - 3  ) throw runtime_error("_2optSwap: start out of bounds.");
    if ( length < 2 || length > sz - 2  ) throw runtime_error(
                (stringstream() << "_2optSwap: length out of bounds: " << sz << "/" << length << " (size/length)").str()   );
    if ( start + length > sz  ) throw runtime_error("_2optSwap: start+length out of bounds.");
#endif
    path_t newpath(path);
    reverse(newpath.begin()+start,newpath.begin()+start+length);
    return path_t(newpath);
}

}// end of anonymous namespace

_2optSingle::_2optSingle(unsigned length):_length(length){}

std::vector<path_t> _2optSingle::operator()(const path_t &path)
{

    const unsigned sz   = static_cast<unsigned>(path.size());
    auto length         = _length;
    if (!length) length = onion::rand_between(2,sz-2);
    auto start          = onion::rand_between(1,sz-length-1);
    auto newpath        = _2optSwap(path, start, length);

    return std::vector<path_t>(1,newpath);
}

_2optAll::_2optAll(unsigned length):_length(length){}

std::vector<path_t> _2optAll::operator()(const path_t &path)
{
    const unsigned sz   = static_cast<unsigned>(path.size());
    auto length         = _length;
    if (!length) length = onion::rand_between(2,sz-2);

    std::vector<path_t> res;
    res.reserve(sz-length-1);

    for(unsigned start = 1; start < sz-length; ++start)
        res.push_back(_2optSwap(path,start,length));

    return std::vector<path_t>( res );
}

path::AbstractObjective::cost_type path::Objective::operator()(const path_t &p)
{
    const auto & sz = p.size();
    path::AbstractObjective::cost_type cost = 0;
    for(unsigned i = 0; i < sz-1; ++i)
        cost += _data.at(p[i]).at(p[i+1]);
    return cost;
}

path::Objective::Objective(const tsp_problem_data_t &d):
    onion::Objective<path_t,tsp_problem_data_t>(d){}


std::vector<path_t> MaskReinsert::operator()(const path_t &path)
{
    const unsigned sz   = static_cast<unsigned>(path.size());
    auto length         = _length;
    if (!length) length = onion::rand_between(1,sz-3);
    auto start          = onion::rand_between(1,sz-length-1);

    path_t mask( path.cbegin()+start, path.cbegin()+start+length );
    path_t rest( sz - length );

    copy(path.cbegin(),path.cbegin()+start,rest.begin());
    copy(path.cbegin()+start+length, path.cend(), rest.begin()+start);

    path_t neighbor(sz);
    vector<path_t> results;
    results.reserve(sz-length-2);
    // ABCDEFGA
    // 01234567 sz 8
    // l=3 s = 3
    // mask = DEF
    //        01234
    // rest = ABCGA ABC + GA
    for(unsigned pos = 1; pos < rest.size(); ++pos)
    {
        if ( pos == start ) continue;
        // ADEFBCGA
        copy(rest.cbegin(),rest.cbegin()+pos,   neighbor.begin());
        copy(mask.cbegin(),mask.cend(),         neighbor.begin()+pos);
        copy(rest.cbegin()+pos,rest.cend(),     neighbor.begin()+pos+length);

        results.push_back(neighbor);
    }

    return vector<path_t>(results);
}
