#ifndef OBSERVER_H
#define OBSERVER_H

#include <vector>

namespace onion{

class Observer{
public:
    virtual ~Observer() = default;
    virtual void update() = 0;
};

class Subject{
public:
    void addObserver(Observer& o){
        for( auto it = _observers.begin(); it != _observers.end(); it++){
            if ( &(it->get()) == &o) return;
        }
        _observers.push_back(o);
    }

    void removeObserver(Observer& o){
        for( auto it = _observers.begin(); it != _observers.end(); it++){
            if ( &(it->get()) == &o){
                _observers.erase(it);
                return;
            }
        }
    }

    void notify(){
        for( auto o : _observers){
            o.get().update();
        }
    }

private:

    std::vector<std::reference_wrapper<Observer>> _observers;
};

}

#endif // OBSERVER_H
