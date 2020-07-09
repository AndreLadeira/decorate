#include "loopcontroller.h"

using namespace onion;
using namespace std;

__Trigger::__Trigger(const std::string &label):LabeledObject(label){}

bool LoopController::running () {

    for( auto trigger : _triggers )
        if (trigger->activated()) {
            _triggerID = trigger->getLabel();
            return false;
        }
    return true;
}

string LoopController::getTrigger() const
{
    return _triggerID;
}

void LoopController::resetTriggers()
{
    for( auto trigger : _triggers )
        trigger->reset();
}
