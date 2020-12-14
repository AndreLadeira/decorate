#ifndef SIMMULATED_ANNEALING_H
#define SIMMULATED_ANNEALING_H

#include "../lib/accept.h"

namespace onion {

namespace sian{


using temp_decay_fcn_t = double (*)(unsigned,const unsigned);

template< typename cost_t,
          typename Compare<cost_t>::compare_fcn_t compare>
class Accept : public NonCopyable
{
public:

    Accept( temp_decay_fcn_t t ):_tempfcn(t){}
    virtual ~Accept() = default;

    virtual  AcceptResult operator()(const cost_t&,
                              const std::vector<cost_t>&) const = 0;
private:

    temp_decay_fcn_t _tempfcn;
};



}// sian

}

#endif // SIMMULATED_ANNEALING_H
