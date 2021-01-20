#ifndef RECORDER_H
#define RECORDER_H

#include <vector>
#include <list>
#include <ostream>
#include "values.h"
#include "observer.h"

namespace onion {

class __Track{
public:
    __Track(std::string name);
    virtual         ~__Track() = default;
    virtual void    record() = 0;
    virtual void    clear() = 0;
    virtual size_t  size() const = 0;
    std::string getName() const;
private:
    std::string _name;
};

template<typename T>
struct TrackStats;

template<typename T>
class Track : public __Track
{
public:
    explicit Track(std::string name, const AValue<T>& source):
        __Track(name),_source(source){}
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
    MultiTrack(std::string name,const AValue<T>& source, Subject& subject):
        __Track(name),_source(source){
        subject.addObserver(*this);
    }
    MultiTrack(std::string name,const AValue<T>& source):
        __Track(name),_source(source){
    }
    virtual ~MultiTrack() = default;

    virtual void record(){
        _track.push_back( _source.getValue() );
    }
    virtual void clear(){
        _record.clear();
        _track.clear();
    }
    // returns the size of the smallest record
    virtual size_t size() const {
        if (_record.size() ){
            auto sz = std::numeric_limits<size_t>::max();
            for (auto & rec: _record )
               if( rec.size() < sz ) sz = rec.size();
            return sz;
        }
        return 0;
    }

    size_t trackCount() const{
       return _record.size();
    }

    virtual std::vector<T> getTrack(unsigned track) const {
        if (track > _record.size() )
            return std::vector<T>();
        else
            return std::vector<T>( _record.at(track).cbegin(), _record.at(track).cend());
    }

    virtual void update(){
        if ( _record.size() && (_track.size() < _record.back().size() ) ){
            // incomplete track
            _track.clear();
            return;
        }
        else{
            _record.push_back(_track);
            _track.clear();
        }
    }

private:

//    friend class Recorder;

//    void removeLastTrack(){
//        if (!_record.size() ) return;
//        else _record.erase( _record.end() - 1 );
//    }

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

           // regularity == 0: record every frame
           // _calls == 0: always record the 1st frame
           // _calls % _regularity == 0 record every _regularity frames
           if ( _regularity == 0 || _calls == 0 || ( (_calls+1) % (_regularity) == 0) ){
               for( auto track : _tracks )
                   track.get().record();
           }
           _calls+= addedCalls;
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

    template<class T>
    Track<T> getTrack(std::string track_name){
        for(const auto & tr: _tracks){
            if (tr.get().getName() == track_name){
                const Track<T>& trref = dynamic_cast<const Track<T>&>(tr.get());
                return Track<T>( trref );
            }
        }
        throw std::runtime_error( std::string("[Recorder::getTrack] - no such track:") + track_name );
    }

private:

    std::vector<std::reference_wrapper<__Track>> _tracks;

    bool      _started;
    unsigned  _regularity;
    unsigned  _calls;

};

}


#endif // RECORDER_H
