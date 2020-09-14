#include "loopcontroller.h"

using namespace onion;
using namespace std;

__Trigger::__Trigger(const std::string &label):LabeledObject(label){}

bool LoopController::operator()() {

    _loopCount++;

    for( auto trigger : _triggers )
        if (trigger->activated()) {
            _triggerID = trigger->getLabel();
            this->reset();
            this->notify();
            return false;
        }
    return true;
}

void LoopController::reset()
{
    _loopCount = 0;

    for( auto trigger : _triggers )
        trigger->reset();
}

string LoopController::getTrigger() const
{
    return _triggerID;
}
