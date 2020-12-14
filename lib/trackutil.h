#ifndef TRACKSTATS_H
#define TRACKSTATS_H

#include <numeric> // accumulate
#include <cmath>   // sqrt
#include <iostream>
#include <iomanip> // setprecision
#include <memory>
#include "recorder.h"

namespace onion{

template<typename T>
struct TrackStats
{
    TrackStats(const std::vector<T>& t):_track(t){}
    TrackStats(const Track<T>& t):_track(t.getTrack()){}

    double average() const {
        if ( _track.size() ){
            T sum = std::accumulate(_track.cbegin(), _track.cend(), T(0) );
            return static_cast<double>(sum) / _track.size();
        }
        else return 0;
    }
    T min() const {
      return *std::min_element(_track.cbegin(), _track.cend());
    }
    T max() const {
      return *std::max_element(_track.cbegin(), _track.cend());
    }
    double stdDev() const {
        if ( _track.size() ){
            double avg = average();
            double accum = 0.0;
            for(const auto& v : _track)
                accum += (v - avg) * (v - avg);
            return std::sqrt( accum / _track.size() );
        }else return 0;
    }

private:

    const std::vector<T> _track;
};

template<typename T>
std::vector<T> getStagnationTrack(const MultiTrack<T>& mt)
{
    std::vector<T> stTrack;
    stTrack.reserve(mt.trackCount());

    for(unsigned i = 0; i < mt.trackCount(); ++i){

        unsigned stagcount = 0;
        auto track = mt.getTrack(i);

        unsigned pos = track.size() - 1;

        while( (pos > 1) && ( track.at(pos) == track.at(pos-1)) ){
            pos--;
            stagcount++;
        }

        stTrack.push_back(stagcount);
    }

    return std::vector<T>(stTrack);
}

template<typename T>
std::vector<T> getAverageTrack(const MultiTrack<T>& mt)
{
    std::vector<T> avgTrack;
    const auto trCount = mt.trackCount();
    const auto trSz = mt.size();

    avgTrack.reserve(trSz);

    for(unsigned val = 0; val < trSz; ++val){

        double avg = 0.0;
        for(unsigned tr = 0; tr < trCount; ++tr)
            avg += mt.getTrack(tr).at(val);
        avg /= trCount;

        avgTrack.push_back(avg);
    }

    return std::vector<T>(avgTrack);
}

template<typename T>
std::vector<T> getMinTrack(const MultiTrack<T>& mt)
{
    std::vector<T> minTrack;
    const auto trCount = mt.trackCount();
    const auto trSz = mt.size();

    minTrack.reserve(trSz);

    for(unsigned val = 0; val < trSz; ++val){

        T min = std::numeric_limits<T>::max();
        for(unsigned tr = 0; tr < trCount; ++tr)
            if ( mt.getTrack(tr).at(val) < min )
                min = mt.getTrack(tr).at(val);
        minTrack.push_back(min);
    }

    return std::vector<T>(minTrack);
}

template<typename T>
std::vector<T> getMaxTrack(const MultiTrack<T>& mt)
{
    std::vector<T> maxTrack;
    const auto trCount = mt.trackCount();
    const auto trSz = mt.size();

    maxTrack.reserve(trSz);

    for(unsigned val = 0; val < trSz; ++val){

        T max = std::numeric_limits<T>::min();
        for(unsigned tr = 0; tr < trCount; ++tr)
            if ( mt.getTrack(tr).at(val) > max )
                max = mt.getTrack(tr).at(val);
        maxTrack.push_back(max);
    }

    return std::vector<T>(maxTrack);
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const TrackStats<T>& ts ){
    os << ts.min() << "\t" << ts.average() << "\t" << ts.max();
    return os;
}
template<typename T>
std::ostream& operator<<(std::ostream& os, const MultiTrack<T>& mt )
{

    const auto trCount = mt.size();
    const auto trSz = mt.getTrack(0).size();

    for(unsigned val = 0; val < trSz; ++val){

        double avg = 0.0;
        T min = std::numeric_limits<T>::max();
        T max = 0;
        for(unsigned tr = 0; tr < trCount; ++tr){
            T v = mt.getTrack(tr).at(val);
            if ( v > max ) max = v;
            if ( v < min ) min = v;
            avg += v;
        }
        os << val << "\t" << min << "\t" << max << "\t" << avg / trCount << "\n";
    }
    return os;
}



template<typename T, typename U>
void printxy(const std::vector<T>& x, const std::vector<T>& y){
    if (x.size() != y.size()) return;

    for(unsigned i = 0; i < x.size(); i++){
        std::cout<< x.at(i) << "\t" << y.at(i) << "\n";
    }

}

template<typename T, typename U>
void printAverages(const MultiTrack<T>& x, const MultiTrack<T>& y){

    if (x.trackCount() != y.trackCount()) return;

    auto xavg = getAverageTrack(x);
    auto yavg = getAverageTrack(y);

    if (xavg.size() != yavg.size()) return;

    printxy<T,U>(xavg,yavg);

}

struct TrackPrinter{

    TrackPrinter() = default;

    struct __track{
        virtual ~__track() = default;
        virtual void print(std::ostream& os, unsigned pos) = 0;
        virtual std::string getName() const = 0;
        virtual unsigned getSize() const = 0;
    };

    template<typename T> struct track: public __track{

        track(std::string name, std::vector<T> t):_name(name),_track(t){}

        virtual void print(std::ostream& os, unsigned pos){
            os << _track.at(pos) << "\t";
        }

        virtual std::string getName() const {
            return _name;
        }

        virtual unsigned getSize() const {
            return _track.size();
        }

    private:
        const std::string _name;
        const std::vector<T> _track;

    };

    template<class T>
    TrackPrinter& add(const Track<T>& t){
        _tracks.push_back( std::make_shared<track<T>>( t.getName(), t.getTrack() ) );
        return *this;
    }

    template<class T>
    TrackPrinter& add(const MultiTrack<T>& t){
           _tracks.push_back( std::make_shared<track<T>>( t.getName() + " - min",   getMinTrack(t) ) );
           _tracks.push_back( std::make_shared<track<T>>( t.getName() + " - max",   getMaxTrack(t) ) );
           _tracks.push_back( std::make_shared<track<T>>( t.getName() + " - avg",   getAverageTrack(t) ) );
           return *this;
    }
    template<class T>
    TrackPrinter& add(const track<T>& t){
        _tracks.push_back( std::make_shared<track<T>>(t) );
        return *this;
    }

    template<class T>
    TrackPrinter& operator<<(const Track<T>& t){
        return add(t);
    }
    template<class T>
    TrackPrinter& operator<<(const MultiTrack<T>& t){
        return add(t);
    }
    template<class T>
    TrackPrinter& operator<<(const track<T>& t){
        return add(t);
    }

    void print(std::ostream& os) const {
        os << std::setprecision(3);
        unsigned maxSz = std::numeric_limits<unsigned>::min();
        for(auto t : _tracks){
            if ( t->getSize() > maxSz) maxSz = t->getSize();
            os << t->getName() << "\t";
        }
        os << "\n";
        for(unsigned i = 0; i < maxSz; i++){
           for(auto t : _tracks){
               if ( i < t->getSize() )
                   t->print(os,i);
           }
           os << "\n";
        }
    }
private:
    std::vector< std::shared_ptr<__track> > _tracks;
};

}

#endif // TRACKSTATS_H
