#ifndef RECORDER_H
#define RECORDER_H

#include <vector>
#include <list>
#include <numeric> // accumulate
#include <cmath>   // sqrt
#include <map>
#include <string>
#include <memory>
#include <functional> // reference_wrapper

#include "abstractvalues.h"

namespace onion{


class _Track{
public:
    virtual ~_Track() = default;
    virtual void record() = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
};

template<typename X, typename Y>
struct Stats;

template<typename X, typename Y>
class Track;

template <typename X, typename Y>
std::ostream& operator<<(std::ostream& os, const Track<X,Y>& record);

template<typename X = unsigned, typename Y = unsigned>
class Track : public _Track
{
public:
    Track(const AValue<X>& xsource, const AValue<Y>& ysource):
        _xsource(xsource),_ysource(ysource){}
    virtual ~Track() = default;

    virtual void record(){
        _xrecord.push_back(_xsource.getValue());
        _yrecord.push_back(_ysource.getValue());
    }
    virtual void clear(){
        _xrecord.clear();
        _yrecord.clear();
    }
    virtual size_t size() const { return _xrecord.size(); }

    virtual std::vector<X> getX() const { return get<X>(_xrecord); }
    virtual std::vector<Y> getY() const { return get<Y>(_yrecord); }

private:

    const AValue<X>& _xsource;
    const AValue<Y>& _ysource;

    std::list<X> _xrecord;
    std::list<Y> _yrecord;

    template<typename V> std::vector<V> get(const std::list<V>& list) const{
        return std::vector<V>(list.cbegin(), list.cend());
    }

    friend std::ostream& operator<<(std::ostream& os, const Track<X,Y>& r);
    friend struct Stats<X,Y>;

};

template<typename X = unsigned, typename Y = unsigned>
struct Stats
{
    Stats(const Track<X,Y>& r):_track(r){}

    double average() const {
        if ( _track.size() ){
            Y sum = std::accumulate(_track._yrecord.cbegin(), _track._yrecord.cend(), Y(0) );
            return static_cast<double>(sum) / _track.size();
        }else return 0;
    }
    Y min() const {
      return *std::min_element(_track._yrecord.cbegin(), _track._yrecord.cend());
    }
    Y max() const {
      return *std::max_element(_track._yrecord.cbegin(), _track._yrecord.cend());
    }
    double stdDev() const {
        if ( _track.size() ){
            double avg = average();
            double accum = 0.0;
            for(const auto&y : _track._yrecord)
                accum += (y - avg) * (y - avg);
            return std::sqrt( accum / _track.size() );
        }else return 0;
    }

private:

    const Track<X,Y>&  _track;
};


template <typename X, typename Y>
std::ostream& operator<<(std::ostream& os, const Track<X,Y>& record){
    auto vx = record.getX();
    auto vy = record.getY();
    for(size_t i = 0; i < record.size(); ++i) os << vx.at(i) << "\t" << vy.at(i) << "\n";
    return os;
}

class Recorder
{
public:
    explicit Recorder(unsigned regularity = 0, bool started = true):
        _started(started),_regularity(regularity),_calls(0){}

    void start()   { _started = true; }

    void stop()    { _started = false; }

    void restart()  {
        _calls      = 0;
        _started    = true;
        for( auto track : _tracks ) track.get().clear();
    }

    void clear(){
        _tracks.clear();
        _calls = 0;
        _started = false;
    }

    bool recording() const
    {
        return _started;
    }

    void record(unsigned addedCalls = 1){
        if ( this->recording() ){
           if ( !_regularity || ( _calls % _regularity == 0) ){
               for( auto track : _tracks )
                   track.get().record();
           }
           _calls+= addedCalls;
        }
    }

    template<class X, class Y>
    void addTrack(Track<X,Y>& track){
           _tracks.push_back(track);
    }

private:

   std::vector<std::reference_wrapper<_Track>> _tracks;

    bool      _started;
    unsigned  _regularity;
    unsigned  _calls;

};




}
#endif // RECORDER_H
