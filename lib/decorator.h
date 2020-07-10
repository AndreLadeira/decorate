#ifndef DECORATOR_H
#define DECORATOR_H

#include <memory>
#include <stdexcept>

namespace onion{

template<class T>
class Decorator
{
public:

    Decorator():_core(new T), _ptr(_core){}
    Decorator(T* t):_core( t ),_ptr(_core){}
    //Decorator(T&& t):_core( std::make_shared<T>(&t) ),_ptr(_core){}
//    Decorator(const T&& t):_ptr(&t){}

    template<typename U> Decorator(U* u):_core( u ),_ptr(_core){}
//    template<typename U, typename V> Decorator(U u, V v):_ptr(new T(u,v)){}

    virtual ~Decorator() = default;
    using ptr_t = std::shared_ptr<T>;
    //using const_ptr_t = const std::shared_ptr<T>;

    template <typename U> void decorate_with(){
        auto p = std::shared_ptr<U>( new U(_ptr) );
        _ptr = p;
    }
    template <typename U, typename V> void decorate_with(const V& v){
        auto p = std::shared_ptr<U>( new U(_ptr,v) );
        _ptr = p;
    }

//    T& core() const {return *_core;}
//    const T& const_core() const {return *_core;}
    ptr_t operator->() const { return _ptr;}
    //T& operator()() const { return *_core;}

    auto operator()(void) -> decltype(T::operator()) { return (*_core)(); }
    template<typename U>
    auto operator()(U& u) -> decltype(T::operator()){ return (*_core)(u); }

    ptr_t dynamicObject() const { return _ptr;}
    T& staticObject() const { return *_core;}

    template<typename U> U& as() const {
        auto p = dynamic_cast<U*>(_ptr.get());
        if (p) return *p;
        else{
            Decorator<T>* ptr = dynamic_cast<Decorator<T>*>(_ptr.get());
            if (ptr) return ptr->as<U>();
            else
                throw std::runtime_error("Decorator::as: bad cast" );
        }
    }
    template<typename U> const U& as_const() const {
        return as<U>();
    }

protected:

    Decorator(ptr_t ptr):_ptr(ptr){}

    ptr_t _core;
    ptr_t _ptr;

};

}
#endif // DECORATOR_H
