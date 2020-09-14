#ifndef TRACKSTATS_H
#define TRACKSTATS_H

#include <numeric> // accumulate
#include <cmath>   // sqrt
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
    stTrack.reserve(mt.size());

    for(unsigned i = 0; i < mt.size(); ++i){

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
    const auto trCount = mt.size();
    const auto trSz = mt.getTrack(0).size();

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
std::ostream& operator<<(std::ostream& os, const MultiTrack<T>& mt )
{

    const auto trCount = mt.size();
    const auto trSz = mt.getTrack(0).size();

    for(unsigned val = 0; val < trSz; ++val){

        double avg = 0.0;
        for(unsigned tr = 0; tr < trCount; ++tr){
            os << mt.getTrack(tr).at(val) << "\t";
            avg += mt.getTrack(tr).at(val);
        }

        os << avg / trCount << "\n";

    }
    return os;
}

}

#endif // TRACKSTATS_H
