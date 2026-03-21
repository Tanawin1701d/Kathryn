//
// Created by tanawin on 25/1/2567.
//

#include "user_event.h"
#include "sim/controller/sim_controller.h"

#include "utility"


namespace kathryn{

    UserEvent::UserEvent(std::function<void(UserEvent&)> active_func,
                         UserEvent* parent,
                         int pri):
            EventBase(parent->get_orchest_cycle(), pri, false),
            _orchestCycle(parent->get_orchest_cycle()),
            _parent(parent),
            _activeFunc(std::move(active_func))
    {
        get_sim_controller_ptr()->add_event(this);
    }

    UserEvent::UserEvent(std::function<void(UserEvent&)> active_func,
               CYCLE sim_cycle,
               int pri):
            EventBase(sim_cycle, pri, false),
            _orchestCycle(sim_cycle),
            _parent(nullptr),
            _activeFunc(std::move(active_func)){
        get_sim_controller_ptr()->add_event(this);
    }

    UserEvent::UserEvent():
    EventBase(0, SIM_USER_PRIO_FRONT_CYCLE, false),
    _orchestCycle(0),
    _parent(nullptr)
    {}



    void UserEvent::operator<<
            (std::function<void(UserEvent&)> sim_behaviour) {

        auto* event = new UserEvent(std::move(sim_behaviour),
                                    this,
                                    _orchestPriority
        );
        _subEvents.push_back(event);
    }


}