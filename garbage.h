/*#ifndef GARBAGE_H
#define GARBAGE_H

#include <iostream>
#include <vector>

objectives: pass state along allowing

    - code reuse (minimize or eliminate code change when changing state)
    - Code that HAS to change: all local classes
    - Code that shoudnt change:
            general program workflow
            added functionality
    - OOP principles:
        separation of concerns (one class one reponsibility)
        clear definitions of reponsibilities among classes
        data protection (restriction of permissions)




struct coreState // read and write
{
   signed current;
   signed best = 1000;
};


 * Definition: defines a contract where developers are required to create
 *             a class for holding MH parameters and another to initialize
 *             them from some data source


class mhParametersState // read only
{
    signed A;
    double B;
    friend class initializeMHparametersDI;
public:
    signed getA(){return A;}
    double getB(){return B;}
};

class initializeMHparametersDI // dependency injection
{
public:
    initializeMHparametersDI(mhParametersState& mhps, istream& is)
    {is>>mhps.A; is>>mhps.B;}
};

class initCurrentState
{
public:
    initCurrentState(coreState& s):_s(s){}
    void operator()(void){
            _s.current = 1000;
    }
private:
    coreState _s;
};

class changeCurrentState
{
public:
    changeCurrentState(coreState& s):_s(s){}
    void operator()(void){
            _s.current -= 1;
    }
private:
    coreState _s;
};

class updateBestState
{
public:
    updateBestState(coreState& s):_s(s){}
    void operator()(void){
            if ( _s.best ) _s.best = _s.current;
    }
private:
    coreState _s;
};

class isFinished
{
public:
    isFinished(coreState& s):_s(s){}
    bool operator()(void){
            if ( !_s.best ) return true;else return false;
    }
private:
    coreState _s;
};

 Good: clear contract, easily creation of required readonly
*       behaviour, pretty basic and straightforward OOP stuff.
*
* Bad: forces developers to create ad-hoc classes for every MH
*      parameters in a specified way they have to know about.
*      (boring to learn easy to forget)
*
*      Error prone (depends on the developer to enforce read-onlyness)
*
*      Standardized practice instead of standardazed interface plus
*      required behaviour
*
*      Classes are coupled to state, but they have to. The presented
*      model defines an interface for each class (a functor) and they
*      dould have been inherited from an abstract base so to keep main
*      unchanged.
*      Templates can cope with the different state/parameters type
*      problem, so the general algorithm code remains unchanged
*      and added functionality works (decorators)
*
*



// observer

// inheritance

// decorator


class algorithmOne // thats a facade!
{
    coreState _cs;
    mhParametersState _mhps;
    initializeMHparametersDI _initMhps; // can change using command pattern
    changeCurrentState _changeCurrent; //
    updateBestState _updateBest; //
    isFinished _isFinished; //

public:

    algorithmOne(istream& is):_initMhps(_mhps,is), _changeCurrent(_cs),
       _updateBest(_cs), _isFinished(_cs){}

     void exec(void)
     { // this cant change
        while(!_isFinished())
        {
            _changeCurrent();
            _updateBest();
        }
    }
};


// main is the program, can change

template<typename T> void xpto( vector<T> data)
{
    cout<< data[1];
}
template<typename T> struct xptoC
{
    xptoC(vector<T> &v):_v(v){}
    vector<T> &_v;
    void exec()
    {
       cout<< _v[0] << endl;
    }
};

// data
std::vector<int> data;
// data box
template<class data_t> class data_box
{
public:
    const data_t & get();
};
// data loaded
// data consumer





int main()
{
//    algorithmOne algoOne(std::cin);
//    algoOne.exec();

//    vector<int> v = {1,2};
//    xptoC<int> xc(v); xc.exec();
}



#endif // GARBAGE_H

            unsigned sz;
            try{
                sz = static_cast<unsigned>( stoul(data_section.at("DIMENSION")) );
            }
            catch(invalid_argument&) {throw runtime_error("tsp_tsplibDataLoader: DIMENSION data is corrupted.");}
            catch(out_of_range&){ throw runtime_error("tsp_tsplibDataLoader: data section is corrupted, DIMENSION is missing.");}

            string type;

            try {
                type = data_section.at("TYPE");
            }
            catch(out_of_range&) {throw runtime_error("tsp_tsplibDataLoader: data section is corrupted, TYPE is missing.");}

            try {
                type = data_section.at("TYPE");
            }
            catch(out_of_range&) {throw runtime_error("tsp_tsplibDataLoader: data section is corrupted, TYPE is missing.");}



*/

/*


*/

/*
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

#include "lib/onion.h"

using namespace std;

struct value{
    string v = "1024";
    operator string () const { return v; }
    template<typename T> T as(){
       stringstream ss(v);
       T v;
       ss >> v;
       return v;
    }
    string as(){ return v;}
};

struct uniqueInt
{
    virtual ~uniqueInt() = default;
    virtual int getInt() const  = 0;
};

struct unique2 : public uniqueInt{
    virtual int getInt() const { return 2; }
};

struct unique3 : public uniqueInt{
    virtual int getInt() const { return 3; }
};

class data
{
public: data() = default;
    string as_string() const { return _data;}
private:
    friend class dataLoader;
    string _data;
};

class dataLoader
{
public:
    dataLoader() = default;
    virtual ~dataLoader() = default;
    virtual void set(data &d) const { d._data = "123"; }
    string get() const { return "123"; }

protected:

    void add(data &d, string s) const {d._data += s;}
    string nonConst() { return "Non Const!"; }

};

template<class T>
class Decorator
{
public:
    Decorator():_ptr(new T){}
    virtual ~Decorator(){ cout<<"Deco delete\n"; delete _ptr;}
    template <typename U> void decorate_with()
    {
        U* p = new U(_ptr);
        _ptr = p;
    }
    const T& get_object() const {return *_ptr;}

    template<typename U> const U& as() const {
        auto p = dynamic_cast<U*>(_ptr);
        if (p) return *p;
        else{
            Decorator<T>* ptr = dynamic_cast<Decorator<T>*>(_ptr);
            if (ptr) return ptr->as<U>();
            else
                throw runtime_error("at Decorator.as - bad cast" );
        }

    }

protected:

    Decorator(T* ptr):_ptr(ptr){}
    T* _ptr;

};

template<class T>
class DecoratorSP
{
public:
    DecoratorSP():_ptr(new T){}
    template <typename U> void decorate_with()
    {
        auto p = shared_ptr<U>( new U(_ptr) );
        _ptr = p;
    }
    const T& const_object() const {return *_ptr;}
    T& object() const {return *_ptr;}

    template<typename U> const U& as() const {
        auto p = dynamic_cast<U*>(_ptr.get());
        if (p) return *p;
        else{
            DecoratorSP<T>* ptr = dynamic_cast<DecoratorSP<T>*>(_ptr.get());
            if (ptr) return ptr->as<U>();
            else
                throw runtime_error("at Decorator.as - bad cast" );
        }

    }



protected:

    DecoratorSP(shared_ptr<T> ptr):_ptr(ptr){}
    shared_ptr<T> _ptr;

};

class dataLoaderDeco456 :
        public dataLoader,
        public Decorator<dataLoader>,
        public unique2
{
    friend class Decorator<dataLoader>;
    dataLoaderDeco456( dataLoader* dl):Decorator<dataLoader>(dl){}

    virtual ~dataLoaderDeco456() { std::cout << "456 dtor ";}
    virtual void set(data &d) const { _ptr->set(d); add(d,"456" );}

public:

    string get() const { return "456"; }
};

class dataLoaderDeco456SP :
        public dataLoader,
        public DecoratorSP<dataLoader>,
        public unique2
{
    friend class DecoratorSP<dataLoader>;
    dataLoaderDeco456SP( shared_ptr<dataLoader> dl):DecoratorSP<dataLoader>(dl){}

    virtual void set(data &d) const { _ptr->set(d); add(d,"456" );}

public:

    string get() const { return "456"; }
};

class dataLoaderDeco678 :
        public dataLoader,
        public Decorator<dataLoader>,
        public unique3
{
    friend class Decorator<dataLoader>;
    dataLoaderDeco678(dataLoader* dl):Decorator<dataLoader>(dl){}
    virtual ~dataLoaderDeco678() { std::cout << "678 dtor ";}
    virtual void set(data &d) const { _ptr->set(d); add(d,"678" );}
public:
    string get() const { return "678"; }
};

class dataLoaderDeco789SP :
        public dataLoader,
        public DecoratorSP<dataLoader>,
        public unique2
{
    friend class DecoratorSP<dataLoader>;
    dataLoaderDeco789SP( shared_ptr<dataLoader> dl):DecoratorSP<dataLoader>(dl){}

    virtual void set(data &d) const { _ptr->set(d); add(d,"456" );}

public:

    string get() const { return "456"; }
};


int main(void)
try
{
    string s = value();

    auto x = value().as<char>();

    auto ss = value().as<string>();

    std::cout<< +x << endl;
    std::cout<< ss << endl;

//    dataLoader *dl = new dataLoader();
//    dataLoaderDeco456 *dl456 = new dataLoaderDeco456(dl);
//    dl = dl456;
//    dataLoaderDeco678 *dl678 = new dataLoaderDeco678(dl);
//    dl = dl678;

//    data d;
//    dl->set(d);

    Decorator<dataLoader> dl;
    dl.decorate_with<dataLoaderDeco456>();
    dl.decorate_with<dataLoaderDeco678>();
    data d;
    dl.get_object().set(d);

    std::cout << d.as_string() << endl;
    std::cout << dl.as<dataLoaderDeco456>().get() << endl;
    std::cout << dl.as<dataLoaderDeco678>().get() << endl;
    std::cout << dl.as<dataLoader>().get() << endl;
    std::cout << dl.as<unique2>().getInt() << endl;
    std::cout << dl.as<unique3>().getInt() << endl;

//    DecoratorSP<dataLoader> dsp;
//    dsp.decorate_with<dataLoaderDeco456SP>();
//    dsp.decorate_with<dataLoaderDeco789SP>();
//    dsp.get_object().set(d);
//    std::cout << d.as_string() << endl;
//    std::cout << dsp.as<dataLoaderDeco456SP>().get() << endl;


}
catch(runtime_error& e){
    cout<< "Run time error: " << e.what() << endl;
}


*/

/*
#ifndef TYPES_H
#define TYPES_H

#include <vector>

namespace onion
{

template <typename T>
class Type
{
public:
   using type = T;
};

template<typename T>
class Array : public Type<T>
{

public:
    Array(unsigned size):_size(size),_array(_size){}
    T& operator[](unsigned pos){
        if ( pos < _size ) return _array.at(pos);
        throw std::out_of_range("onion::Array");
    }
    Array& operator=(const Array& rhs){
        if ( rhs._size == _size){
            this->_row = rhs._row;
            return *this;
        }
        else
            std::runtime_error("Array::operator=: dimensions mismatch.");
    }
private:
    unsigned _size;
    std::vector<T> _array;
};

template<typename T>
class Matrix : public Type<T>
{
public:
    Matrix(unsigned rows, unsigned cols):
        _rows(rows),_cols(cols),_matrix(_rows,_cols){}

    Matrix& operator=(const Matrix& rhs){
        if (rhs._cols == _cols && rhs._rows == _rows){
            this->_matrix = rhs._matrix;
            return *this;
        }
        else
            throw std::runtime_error("Matrix::operator=: dimensions mismatch.");
    }

    Matrix(const Matrix& rhs) = default;

    Array<T>& operator[](unsigned row){
        if (row < _rows) return _matrix.at(row);
        throw std::out_of_range("onion::Matrix");
    }

private:
    unsigned _rows;
    unsigned _cols;
    std::vector<Array<T>> _matrix;

};

using bit = bool;
using arrayOfBits = Array< bit >;
using arrayOfUnsigned = Array< unsigned >;
using arrayOfInts = Array< signed >;
using matrixOfBits = Matrix< bool >;
using matrixOfUnsigned = Matrix< unsigned >;
using matrixOfInts = Matrix< signed >;

}

#endif // TYPES_H

*/


/*
 *
 *


        updateOuter.as<tsp::path::UpdateRecorder>().clear();

        updateOuter.as<tsp::path::UpdateRecorder>().addRecord(
                    "best x obj. fcn. calls",
                    Record<unsigned,unsigned>(objective.as<Counter>(),BoundValue<unsigned>(best_cost)) );

        updateOuter.as<tsp::path::UpdateRecorder>().addRecord(
                    "best x time",
                    Record<double,unsigned>(timer,BoundValue<unsigned>(best_cost)) );

        updateOuter.as<tsp::path::UpdateRecorder>().start();
        updateOuter.as<tsp::path::UpdateRecorder>().record(); // record initial values

//        objective.as<tsp::path::ObjectiveRecorder>().clear();

//        objective.as<tsp::path::ObjectiveRecorder>().addRecord(
//                    "best x obj. fcn. calls",
//                    Record<unsigned,unsigned>(objective.as<Counter>(),BoundValue<unsigned>(best_cost)) );

//        objective.as<tsp::path::ObjectiveRecorder>().addRecord(
//                    "best x time",
//                    Record<double,unsigned>(timer,BoundValue<unsigned>(best_cost)) );

//        objective.as<tsp::path::ObjectiveRecorder>().start();
//        objective.as<tsp::path::ObjectiveRecorder>().record(); // record initial values

        updateInner.as<tsp::path::UpdateRecorder>().clear();
        updateInner.as<tsp::path::UpdateRecorder>().addRecord(
                    "best x int. loop calls",
                    Record<unsigned,unsigned>(innerloop.as<Counter>(),BoundValue<unsigned>(current_cost)) );


                    */

/*

    cout<< "\nExecution stop triggered by       : " << outerloop.core().getTrigger() << endl;
    cout<< "Execution time                      : " << fixed << setprecision(4) << timer.getValue() << "s\n";
    cout<< "Final result                        : " << best_cost << endl;
    cout<< "Final path                          : " << best;

 */


/*

#ifndef VALUES_H
#define VALUES_H

#include <memory>
#include <ctime>

namespace onion {


template<typename T>
struct AValue{
    virtual ~AValue() = default;
    virtual T getValue() const = 0;
    virtual void setValue(const T& v) = 0;
};


struct AResettable
{
    AResettable(bool locked = false);
    virtual ~AResettable();
    virtual void reset(){
        if (!_locked) this->doReset();
    }
    virtual void hardReset(){
        this->doReset();
    }

protected:

    virtual void doReset() = 0;

private:

    bool _locked;
};

template<typename T>
struct Value:
        public AValue<T>{

    explicit Value(T v0 = T(0)):_v(v0){}
    virtual ~Value() = default;
    virtual T getValue() const { return _v;}
    virtual void setValue(const T& v) {_v = v;}

protected:

    T _v;

};

//template<typename T>
//struct ResettableValue :
//        public Value<T>,
//        public AResettable
//{
//    explicit ResettableValue(T v = T(0), bool locked = false):
//        Value<T>(v),AResettable(locked),_v0(v){}

//    virtual ~ResettableValue() = default;

//    virtual void doReset(){
//           Value<T>::_v = _v0;
//    }

//private:

//    T _v0;
//};

template<typename T>
struct RefValue:
        public AValue<T>
{
    explicit RefValue(T& v):_vref(v){}
    //explicit RefValue(T&& v):_vref(v){}
    virtual ~RefValue() = default;
    virtual T getValue(void) const {
        return _vref;
    }
    virtual void setValue(const T& v){
        _vref = v;
    }

protected:

    T& _vref;
};

//struct ACounter{
//    virtual void count(unsigned amount) = 0;
//    virtual ~ACounter();
//};

//struct Counter :
//        public Value<unsigned>,
//        public ACounter
//{
//    explicit Counter(unsigned start = 0);
//    virtual ~Counter() = default;
//    void count(unsigned amount = 1);

//};

//struct ResettableCounter :
//        public ResettableValue<unsigned>,
//        public ACounter{

//    explicit ResettableCounter(unsigned start = 0, bool locked = false):
//        ResettableValue<unsigned>(start,locked),_accumulated(0){}
//    virtual ~ResettableCounter() = default;
//    void count(unsigned amount = 1);
//    unsigned getAccumulated();
//private:
//    unsigned _accumulated;
//};

struct Counter :
        public Value<unsigned>
{
    explicit Counter(unsigned start = 0);
    virtual ~Counter() = default;
    void count(unsigned amount = 1);
    void restart();
    unsigned get();
    unsigned getTotal();
private:
    unsigned _total;
};


}


template< typename solution_t,
          typename cost_t,
          typename Compare<cost_t>::compare_fcn_t c>
class UpdateLDRecorder :
        public Updater<solution_t,cost_t, c>,
        public Recorder,
        public OnionLayer<Updater<solution_t,cost_t,c>>
{
public:
    using OnionLayerBase = OnionLayer<Updater<solution_t,cost_t,c>>;
    using Updater = Updater<solution_t,cost_t, c>;

    UpdateLDRecorder(typename OnionLayerBase::core_ptr_t next, unsigned regularity = 1):
        Updater("Local Data Recorder"),
        Recorder(regularity),OnionLayerBase(next),
        _candidate_cost(0),_bsf_cost(0),
        _candidate_tr("ULR.candidate", _candidate_cost  ),
        _bsf_tr("ULR.bsf", _bsf_cost  )
    {
        //this->addTrack ( _local_tr );
    }

    virtual bool operator()(solution_t& bestSoFar,
                        cost_t& bsfCost,
                        const solution_t& candidate,
                        const cost_t candidateCost )
    {
        auto result = (*this->_next)(bestSoFar,bsfCost,candidate, candidateCost);
        _candidate_cost.setValue(candidateCost);
        this->record();
        return result;
    }

    template<class T>
    Track<T> getLocalTrack(){
        return this->template getTrack<T>("local");
    }

private:

    Value<cost_t> _candidate_cost;
    Value<cost_t> _bsf_cost;
    MultiTrack<cost_t> _candidate_tr;
    MultiTrack<cost_t> _bsf_tr;
};


#endif // VALUES_H


  */
