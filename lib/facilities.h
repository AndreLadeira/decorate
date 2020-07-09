#ifndef FACILITY_H
#define FACILITY_H

namespace omh {
class NonCopyable
{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable( const NonCopyable& ) = delete;
    NonCopyable& operator=( const NonCopyable& ) = delete;
};
}


#endif // FACILITY_H
