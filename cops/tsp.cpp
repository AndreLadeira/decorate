#include "tsp.h"
#include <regex>
#include <algorithm>
#include <map>
#include <random>
#include <sstream>
#include <cassert>
#include <cmath>
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
    virtual problem_data_t load(std::istream& is, unsigned sz) const = 0;
    virtual ~DataLoader() = default;
};

struct : public DataLoader
{
    problem_data_t load(std::istream& is, unsigned sz) const
    {
        problem_data_t data(sz,sz);

        for(unsigned i = 0; i < sz; i++)
           for(unsigned j = 0; j < sz; j++){
                is >> data[i][j];
           }

        return problem_data_t( data );
    }
} static tspExplicitLoader;

struct : public DataLoader
{
    problem_data_t load(std::istream& is, unsigned sz) const
    {

        std::vector<double> x(sz,0);
        std::vector<double> y(sz,0);

        int pt = 0;
        for(unsigned i = 0; i < sz; ++i)
            is >> pt >> x[i] >> y[i];

        problem_data_t data(sz,sz);

        for(unsigned i = 0; i < sz; ++i)
            for(unsigned j = 0; j < sz; ++j){
                if ( i != j){
                    double dx = (x[i]-x[j]);
                    double dy = (y[i]-y[j]);

                    data[i][j] = data[j][i] = static_cast<unsigned>(
                            std::round( std::sqrt( dx*dx + dy*dy ) ) );
                }
                else{
                   data[i][j] = data[j][i] =
                           std::numeric_limits<unsigned>::max();
                }
            }
        return problem_data_t( data );
    }
} static tspEuc2DLoader;


struct data_loader_id {
    string type;
    string edge_weight_type;
    string edge_weight_format;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-member-function"
#endif

    bool operator<(const data_loader_id& rhs) const{
        return (type + edge_weight_type + edge_weight_format) <
               ( rhs.type + rhs.edge_weight_type + rhs.edge_weight_format);
    }

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
    string to_string(){
        return type + "/" + edge_weight_type + "/" + edge_weight_format;
    }
};

static map<data_loader_id, const DataLoader& > DataLoaders = {
    { {"ATSP", "EXPLICIT", "FULL_MATRIX" }, tspExplicitLoader },
    { { "TSP", "EXPLICIT", "FULL_MATRIX" }, tspExplicitLoader },
    { { "TSP", "EUC_2D", ""              }, tspEuc2DLoader }
};

}// end of anonymous namespace


problem_data_t tsplibDataLoader::operator()(istream& is)
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

path_t path::CreateRandom::operator()()
{
    path_t path(_data.size()+1);
    // create a random path

    std::iota(path.begin(),path.end()-1,0);

    // begin+1 / end-1 to make paths always starting and ending at city 0,
    // with no loss of generality

    std::shuffle(path.begin()+1,path.end()-1,onion::get_random_engine());

    return path_t(path);
}

path_t CreateGreedy::operator()()
{
    // creates a path, always choosing the least-distance for each city
    static const unsigned  n = static_cast<unsigned>( _data.size() );
    static path_t          visited(n+1,0);

    auto city       = onion::rand_between(0,n-1);
    visited.at(0)   = city;

    for(unsigned i = 1; i < n; i++){
        // get the lest cost, not visited yet, option from current city
        unsigned least_cost         = std::numeric_limits<unsigned>::max();
        unsigned least_cost_city    = 0;

        for(unsigned j = 0; j < n; j++){
           auto end = visited.begin()+i;
           if ( std::find(visited.begin(), end, j ) != end ) continue;

           if ( _data.at(city).at(j) < least_cost ){
               least_cost = _data.at(city).at(j);
               least_cost_city = j;
           }
        }
        visited.at(i)   = least_cost_city;
        city            = least_cost_city;
    }

    static path_t res(n+1,0);

    if ( visited.at(0) != 0 ){

        auto it = std::find(visited.begin(),visited.end(),0);
        auto len = std::distance(it,visited.end());

        std::copy(it,visited.end(),res.begin());
        std::copy(visited.begin(),it,res.begin()+len-1);
    }
    else{
        res = visited;
    }

    return res;
}

std::ostream& path::operator<<(std::ostream &os, const path_t &path)
{
    for(auto city : path){
        os<< city << " ";
    }
    os << "\n";
    return os;
}

//bitmatrix_t bitmatrix::CreateRandom::operator()(){
//    bitmatrix_t matrix(_size,_size);

//    return bitmatrix_t( to_bitmatrix( path::CreateRandom(_size)() ));
//}


//path_t onion::cops::tsp::to_path(const bitmatrix_t & matrix)
//{
//    const auto& sz = matrix.size();
//    path_t path(sz+1);
//    unsigned current = 0;
//    for(unsigned i = 1; i < sz; i++)
//        for(unsigned j = 0; j < sz; j++)
//            if ( matrix.at(current).at(j) )
//            {
//                path.at(i) = j;
//                current = j;
//                break;
//            }
//    return path_t(path);
//}

//bitmatrix_t onion::cops::tsp::to_bitmatrix(const path_t & path)
//{
//    const auto& sz = path.size();
//    bitmatrix_t matrix(sz-1,sz-1);
//    unsigned prev = 0;
//    for(unsigned i = 1; i < sz; ++i )
//    {
//        unsigned next = path.at(i);
//        matrix.at(prev).at(next) = true;
//        prev = next;
//    }
//    return bitmatrix_t(matrix);
//}

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

//_2optSingle::_2optSingle(unsigned length):_length(length){}

//std::vector<path_t> _2optSingle::operator()(const path_t &path)
//{

//    const unsigned sz       = static_cast<unsigned>(path.size());
//    auto length             = _length;
//    if (length < 2) length  = onion::rand_between(2,sz-2);
//    auto start              = onion::rand_between(1,sz-length-1);
//    auto newpath            = _2optSwap(path, start, length);

//    return std::vector<path_t>(1,newpath);
//}

//_2opt::_2opt(unsigned length):_length(length){}

//std::vector<path_t> _2opt::operator()(const path_t &path)
//{
//    const unsigned sz       = static_cast<unsigned>(path.size());
//    auto length             = _length;
//    if (length < 2 ) length = onion::rand_between(2,sz-2);

//    std::vector<path_t> res;
//    res.reserve(sz-length-1);

//    for(unsigned start = 1; start < sz-length; ++start)
//        res.push_back(_2optSwap(path,start,length));

//    return std::vector<path_t>( res );
//}

std::vector<path_t> _2opt::operator()(const path_t &path)
{
    static const size_t n = static_cast<unsigned>(path.size()) - 1;

    static std::vector<path_t> result( (n-1)*(n-2)/2, path_t(n,0));

    // BI-CLASSIC impl. from Hansen, Mladenovic, 2006

    size_t count = 0;
    for (unsigned i = 1; i < n - 1 ; i++){
        //auto n1 = n - 1;
        //if ( i == 0 ) n1 = n1 - 1;
        for(unsigned j = i+2; j <= n; j++){
            result[ count++ ] = _2optSwap(path,i,j-i);
        }
    }
    return std::vector<path_t>(result);
}

path::Objective::cost_type path::tspObjective::operator()(const path_t &p)
{
    const auto & sz = p.size();
    path::Objective::cost_type cost = 0;
    for(unsigned i = 0; i < sz-1; ++i)
        cost += _data->at(p[i]).at(p[i+1]);
    return cost;
}
//#ifdef __DEBUG__
//namespace{
//path_t  mask_reinsert(const path_t &path, unsigned start, unsigned length, unsigned pos){

//    const unsigned sz   = static_cast<unsigned>(path.size());

//    path_t mask( path.cbegin()+start, path.cbegin()+start+length );
//    path_t rest( sz - length );

//    copy(path.cbegin(),path.cbegin()+start,rest.begin());
//    copy(path.cbegin()+start+length, path.cend(), rest.begin()+start);

//    path_t neighbor(sz);

//    // ADEFBCGA
//    copy(rest.cbegin(),rest.cbegin()+pos,   neighbor.begin());
//    copy(mask.cbegin(),mask.cend(),         neighbor.begin()+pos);
//    copy(rest.cbegin()+pos,rest.cend(),     neighbor.begin()+pos+length);
//    return neighbor;
//}
//}
//#endif

std::vector<path_t> RemoveReinsert::operator()(const path_t &path)
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


//path::delta::mask_tranf_t path::delta::MaskReinsert::operator()(const path_t &path)
//{
//    static const unsigned   n       = static_cast<unsigned>(path.size());
//    auto                    length  = _length;

//    if (!length) length = onion::rand_between(1,n-3);
//    auto start          = onion::rand_between(1,n-length-1);

//    vector<delta::mask_tranf_t> results;
//    results.reserve( n - length - 2 );

//    delta::mask_tranf_t tr = { start, length, std::vector<unsigned>( n - length - 2, 0) };
//    size_t trcount = 0;

//    for(unsigned pos = 1; pos < n - length; ++pos){
//        if ( pos == start ) continue;
//        tr._positions[trcount++] = pos;
//    }

//    return delta::mask_tranf_t(tr);
//}

//path::delta::Objective::cost_type
//path::delta::tspObjective::operator()(const path_t& p, const delta::mask_tranf_t& tr)
//{
//    const auto & trcount    = tr._positions.size();
//    const auto start        = tr._mask_start;
//    const auto len          = tr._mask_length;

//    Objective::cost_type costs(trcount,0);
//    // 0 1 2 3 4 5 6
//    // A B C D E F G
//    // start = 2 (C), length = 2 (CD)
//    // cinto = cBC
//    // couto = cDE
//    // cconn = cBE
//    auto cinto      = _data.at( p.at(start-1) ).at( p.at(start) );
//    auto couto      = _data.at( p.at(start+len-1) ).at( p.at(start+len) );
//    auto cconn      = _data.at( p.at(start-1) ).at( p.at(start+len) );

//    // dtcost = cost of ABEFG
//    int dtcost      = static_cast<int>(path::tspObjective(_data)(p) - cinto - couto + cconn);

//    size_t count = 0;
//    for(const auto trpos : tr._positions ){
//        // i = 1
//        // dtcost - cAB + cAC + cDB > A CD BEFG
//        costs[count++] = dtcost -
//                static_cast<int>( _data.at( p.at(trpos-1) ).at( p.at(trpos) ) )+
//                static_cast<int>( _data.at( p.at(trpos-1) ).at( p.at(start)) ) +
//                static_cast<int>( _data.at( p.at(start+len-1) ).at( p.at(trpos) ) );
//#ifdef __DEBUG__
//        assert( path::tspObjective(_data)( mask_reinsert(p, start, len, trpos)) ==
//                static_cast<unsigned>(costs[count-1]) );
//#endif
//    }
//    return costs;

//}


