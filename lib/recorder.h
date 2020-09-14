#ifndef RECORDER_H
#define RECORDER_H

#include <vector>
#include <list>
#include "values.h"
#include "observer.h"

namespace onion {

class __Track{
public:
    virtual ~__Track() = default;
    virtual void record() = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
};

template<typename T>
struct TrackStats;

template<typename T>
class Track : public __Track
{
public:
    explicit Track(const AValue<T>& source):
        _source(source){}
    virtual ~Track() = default;

    virtual void record(){
        _record.push_back(_source.getValue());
    }
    virtual void clear(){
        _record.clear();
    }
    virtual size_t size() const {
        return _record.size();
    }

    virtual std::vector<T> getTrack() const {
        return std::vector<T>( _record.cbegin(), _record.cend());
    }

private:

    friend struct TrackStats<T>;

    const AValue<T>& _source;
    std::list<T> _record;

};

template<typename T>
class MultiTrack :
        public __Track,
        public Observer
{
public:
    explicit MultiTrack(const AValue<T>& source, Subject& trigger):
        _source(source){
        trigger.add(*this);
    }
    virtual ~MultiTrack() = default;

    virtual void record(){
        _track.push_back( _source.getValue() );
    }
    virtual void clear(){
        _record.clear();
        _track.clear();

    }
    virtual size_t size() const {
            return _record.size();
    }

    virtual std::vector<T> getTrack(unsigned track) const {
        if (track > _record.size() )
            return std::vector<T>();
        else
            return std::vector<T>( _record.at(track).cbegin(), _record.at(track).cend());
    }

    virtual void update(){
        if ( _track.size() != 0 ) {
            _record.push_back(_track);
            _track.clear();
        }
    }

private:

    const AValue<T>& _source;
    std::vector< std::list<T> > _record;
    std::list<T> _track;
};

class Recorder
{
public:
    explicit Recorder(unsigned regularity = 0, bool started = false):
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

    bool recording() const{
        return _started;
    }

    void record(unsigned addedCalls = 1){

        if ( this->recording() ){
            _calls+= addedCalls;
           if ( _regularity == 0 || ( _calls % _regularity == 0) ){
               for( auto track : _tracks )
                   track.get().record();
           }
        }
    }

    template<class T>
    void addTrack(Track<T>& track){
           _tracks.push_back(track);
    }
    template<class T>
    void addTrack(MultiTrack<T>& track){
           _tracks.push_back(track);
    }

private:

    std::vector<std::reference_wrapper<__Track>> _tracks;

    bool      _started;
    unsigned  _regularity;
    unsigned  _calls;

};

}


#endif // RECORDER_H
