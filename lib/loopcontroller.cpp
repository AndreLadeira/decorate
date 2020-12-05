#include "loopcontroller.h"

using namespace onion;
using namespace std;

__StopCondition::__StopCondition(const std::string &label):LabeledObject(label){}

LoopController::LoopController(const string label, unsigned maxLoops)
    :_loopCount(0){
    if (maxLoops != 0)
        this->addStopCondition( StopCondition<>(label, this->_loopCount, maxLoops),
                                ON_STOP::RESET );
}

bool LoopController::operator()() {

    // stop conditions stop
    for( auto stopCondition : _stopConditions )
        if (stopCondition._ptr->satisfied()) {
            _stopConditionLabel = stopCondition._ptr->getLabel();
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
        if ( sc._stop_action == ON_STOP::RESET ) sc._ptr->reset();
    for( auto obj : _resObjects ) obj.get().reset();
}

string LoopController::getStopCondition() const
{
    return _stopConditionLabel;
}
