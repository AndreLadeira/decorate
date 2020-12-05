#include "loopcontroller.h"

using namespace onion;
using namespace std;

__StopCondition::__StopCondition(const std::string &label):LabeledObject(label){}

LoopController::LoopController(const string label, unsigned maxLoops)
    :_loopCount(0){
    if (maxLoops != 0)
        this->addStopCondition( StopCondition<>(label, this->_loopCount, maxLoops) );
}

bool LoopController::operator()() {

    // stop conditions stop
    for( auto stopCondition : _stopConditions )
        if (stopCondition.ptr->satisfied()) {
            _stopConditionLabel = stopCondition.ptr->getLabel();
            this->reset();
            this->notify();
            return false;
        }

    this->_loopCount++;

    return true;
}

void LoopController::reset()
{
    for( auto sc : _stopConditions )
        if ( sc.owner == SC_OWNER::THIS ) sc.ptr->reset();
}

string LoopController::getStopCondition() const
{
    return _stopConditionLabel;
}
