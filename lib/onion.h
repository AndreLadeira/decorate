#ifndef ONION_H
#define ONION_H

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <string>


namespace onion{

template <class CoreType>
class Onion;

template <class CoreType>
class OnionLayer{
protected:
    friend class Onion<CoreType>;
    using core_ptr_t = std::shared_ptr<CoreType>;
    OnionLayer(core_ptr_t next):_next(next){}
    core_ptr_t _next;
};

template <class CoreType, class... Args> auto InvokeFunctor(Args&... args)
-> decltype ( static_cast<CoreType>(nullptr)->operator()(args...) )
{}

template <class CoreType> auto InvokeFunctor(const CoreType& _ptr)
-> decltype ( static_cast<CoreType>(nullptr)->operator()() )
{return _ptr->operator()();}

template <class CoreType,class T> auto InvokeFunctor(const CoreType& _ptr, const T &t)
-> decltype ( static_cast<CoreType>(nullptr)->operator()(t) )
{return _ptr->operator()(t);}

//template <class CoreType,class T, class U, class... Args >
//auto InvokeFunctor(const CoreType& _ptr, const T &t, const U &u, const Args&...)
//-> decltype ( static_cast<CoreType>(nullptr)->operator()(t,u) )
//{return _ptr->operator()(t,u);}

template <class CoreType,class T, class U, class... Args >
auto InvokeFunctor(const CoreType& _ptr, T& t, U& u, const Args&...)
-> decltype ( static_cast<CoreType>(nullptr)->operator()(t,u) )
{return _ptr->operator()(t,u);}

template <class CoreType,class T, class U, class V, class... Args >
auto InvokeFunctor(const CoreType& _ptr, T& t, U& u, V& v, const Args&...)
-> decltype ( static_cast<CoreType>(nullptr)->operator()(t,u,v) )
{return _ptr->operator()(t,u,v);}

template <class CoreType,class T, class U, class V, class W, class... Args >
auto InvokeFunctor(const CoreType& _ptr, T& t, U& u, V& v, W& w, const Args&...)
-> decltype ( static_cast<CoreType>(nullptr)->operator()(t,u,v,w) )
{return _ptr->operator()(t,u,v,w);}


template <class CoreType>
class Onion{
public:

    using core_ptr_t = std::shared_ptr<CoreType>;

    explicit Onion(core_ptr_t core):
        _core(core), _outerLayer(_core){

    }

    Onion():
        _core( std::make_shared<CoreType>() ), _outerLayer(_core){

    }

    template<class LayerType>
    std::shared_ptr<LayerType> addLayer(){

        auto newlayer = std::make_shared<LayerType>(_outerLayer);
        _outerLayer = newlayer;
        return newlayer;

    }

//    template<class LayerType, class LayerParamType>
//    std::shared_ptr<LayerType> addLayer(LayerParamType& p){

//        auto newlayer = std::make_shared<LayerType>(_outerLayer)(p);
//        _outerLayer = newlayer;
//        return newlayer;

//    }

    template<class LayerType, class LayerParamType>
    std::shared_ptr<LayerType> addLayer(LayerParamType p){

        auto newlayer = std::make_shared<LayerType>(_outerLayer,p);
        _outerLayer = newlayer;
        return newlayer;
    }

    template<class... Args>
    auto operator()(Args&... args) -> decltype ( InvokeFunctor<core_ptr_t>(nullptr,args...) ){
        return InvokeFunctor<core_ptr_t>(_outerLayer,args...);
    }

//    template<class C> C& as() const {
//        return as<C>(_outerLayer.get());
//    }

    CoreType& core(){
        return *_core;
    }

    using Layer = OnionLayer<CoreType>;

    std::string getLayers( CoreType* ptr = nullptr ){

        if (!ptr) ptr = _outerLayer.get();

       Layer* lptr = dynamic_cast< OnionLayer<CoreType>* >(ptr);
       if (!lptr)
           return ptr->getLabel();
       else
           return ptr->getLabel() + ";" + getLayers(lptr->_next.get());

    }


//    template<class C> C& as(CoreType* ptr) const {
//        C* cptr = dynamic_cast<C*>(ptr);
//        if (cptr) return *cptr;

//        // here, the pointer can point to a layer or to the core
//        // if points to a layer, keep recurring
//        Layer* lptr = dynamic_cast< OnionLayer<CoreType>* >(ptr);

//        if (lptr) return as<C>( lptr->_next.get() );
//        else throw std::runtime_error("Onion::as: bad cast" );
//    }

    const core_ptr_t _core;
    core_ptr_t _outerLayer;

};


}
#endif // ONION_H
