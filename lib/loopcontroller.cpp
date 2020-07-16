#include "loopcontroller.h"

using namespace onion;
using namespace std;

__Trigger::__Trigger(const std::string &label):LabeledObject(label){}

<<<<<<< HEAD
bool LoopController::running() {
=======
bool LoopController::operator()() {
>>>>>>> functors

    for( auto trigger : _triggers )
        if (trigger->activated()) {
            _triggerID = trigger->getLabel();
            return false;
        }
    return true;
}

void LoopController::reset()
{
    for( auto trigger : _triggers )
        trigger->reset();
}

void LoopController::hardReset()
{
    for( auto trigger : _triggers )
        trigger->hardReset();
}

string LoopController::getTrigger() const
{
    return _triggerID;
}
