#ifndef ONION_H
#define ONION_H

#include <memory>
#include <stdexcept>
#include <type_traits>

namespace onion{

template <class CoreType>
class Onion;

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

template <class CoreType,class T, class U, class... Args >
auto InvokeFunctor(const CoreType& _ptr, const T &t, const U &u, const Args&...)
-> decltype ( static_cast<CoreType>(nullptr)->operator()(t,u) )
{return _ptr->operator()(t,u);}

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

    explicit Onion(core_ptr_t core): _core(core), _outerLayer(_core){}
    Onion(): _core( new CoreType ), _outerLayer(_core){}

    template<class LayerType>
    Onion<CoreType>& addLayer(){

        auto newlayer = std::make_shared<LayerType>(_outerLayer);
        _outerLayer = newlayer;
        return *this;
    }

    template<class LayerType, class LayerParamType>
    Onion<CoreType>& addLayer(LayerParamType& p){

        auto newlayer = std::make_shared<LayerType>(_outerLayer)(p);
        _outerLayer = newlayer;
        return *this;
    }

    template<class LayerType, class LayerParamType>
    Onion<CoreType>& addLayer(LayerParamType p){

        auto newlayer = std::make_shared<LayerType>(_outerLayer,p);
        _outerLayer = newlayer;
        return *this;
    }

    template<class... Args>
    auto operator()(Args&... args) -> decltype ( InvokeFunctor<core_ptr_t>(nullptr,args...) ){
        return InvokeFunctor<core_ptr_t>(_outerLayer,args...);
    }

    template<class C> C& as() const {
        return as<C>(_outerLayer.get());
    }

    CoreType& core(){
        return *_core;
    }

private:

    template<class C> C& as(CoreType* ptr) const {
        C* cptr = dynamic_cast<C*>(ptr);
        if (cptr) return *cptr;
        OnionLayer<CoreType>* lptr = dynamic_cast< OnionLayer<CoreType>* >(ptr);
        if (lptr) return as<C>( lptr->_next.get() );
        else throw std::runtime_error("Onion::as: bad cast" );
    }

    const core_ptr_t _core;
    core_ptr_t _outerLayer;

};


}
#endif // ONION_H
