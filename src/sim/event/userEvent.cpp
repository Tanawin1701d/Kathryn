//
// Created by tanawin on 25/1/2567.
//

#include "userEvent.h"
#include "sim/controller/simController.h"

#include <utility>


namespace kathryn{

    UserEvent::UserEvent(std::function<void(UserEvent&)> activeFunc,
                         UserEvent* parent,
                         int pri):
            EventBase(parent->getOrchestCycle(), pri),
            _orchestCycle(parent->getOrchestCycle()),
            _parent(parent),
            _activeFunc(std::move(activeFunc))
    {
        getSimController()->addEvent(this);
    }

    UserEvent::UserEvent():
    EventBase(0, SIM_USER_PRIO_FRONT_CYCLE),
    _orchestCycle(0),
    _parent(nullptr)
    {}



    void UserEvent::operator<<
            (std::function<void(UserEvent&)> simBehaviour) {

        auto* event = new UserEvent(std::move(simBehaviour),
                                    this,
                                    _orchestPriority
        );
        _subEvents.push_back(event);
    }


}