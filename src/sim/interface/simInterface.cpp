//
// Created by tanawin on 25/1/2567.
//

#include "simInterface.h"
#include "model/hwComponent/module/module.h"


namespace kathryn{

    /**user sim agent*/

    SimInterface::UserSimAgent::UserSimAgent(SimInterface* master):
    _master(master)
    {assert(master != nullptr);}

    SimInterface::UserSimAgent&
    SimInterface::UserSimAgent::operator<<(std::function<void(void)> simBehaviour) {
        auto*  event =  new UserEvent(simBehaviour,
                                      _master->_curUserDescCycle,
                                      SIM_USER_PRIO);
        _master->_UserSimEvents.push_back(event);
        return *this;
    }

    /**sim interface*/



    SimInterface::SimInterface():
    _ModuleSimEvent(
            new ModuleSimEvent(getGlobalModulePtr(),rstWire,startNode)
    ){}


    SimInterface::~SimInterface() {
        delete _ModuleSimEvent;
        /**no need to delete user event because sim controller will delete it */
    }

    void SimInterface::simStart() {
        /***module sim Event is auto insert to event*/
        /****/


    }


}