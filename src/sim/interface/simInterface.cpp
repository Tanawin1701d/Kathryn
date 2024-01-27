//
// Created by tanawin on 25/1/2567.
//

#include "simInterface.h"



namespace kathryn{

    /**user sim agent*/

    SimInterface::UserSimAgent::UserSimAgent(SimInterface* master):
    _master(master)
    {assert(master != nullptr);}

    SimInterface::UserSimAgent&
    SimInterface::UserSimAgent::operator<<(std::function<void(void)> simBehaviour) {
        auto*  event =  new UserEvent(simBehaviour,
                                      _master->_nextUserDescCycle,
                                      SIM_USER_PRIO);
        _master->_UserSimEvents.push_back(event);
        return *this;
    }

    /**
     *
     * sim interface
     *
     * */



    SimInterface::SimInterface(CYCLE limitCycle, std::string vcdFilePath):
            _vcdWriter(new VcdWriter(vcdFilePath)),
            _nextUserDescCycle(0),
            _limitCycle(limitCycle)
    {
        _ModuleSimEvent = new ModuleSimEvent(getGlobalModulePtr(),rstWire,_vcdWriter);
        SimController* simCtrl = getSimController();
        assert(simCtrl != nullptr);
        simCtrl->setLimitCycle(limitCycle);
    }

    SimInterface::~SimInterface() {
        delete _ModuleSimEvent;
        /**no need to delete user event because sim controller will delete it */
    }

    void SimInterface::simStart() {
        /***module sim Event is auto insert to event*/
        /****/
        describe();
        SimController* simCtrl = getSimController();
        assert(simCtrl != nullptr);
        simCtrl->simStart();

    }

    void SimInterface::incCycle(CYCLE inCycle){
        _nextUserDescCycle += inCycle;
    }

    void SimInterface::setCycle(CYCLE stCycle){
        _nextUserDescCycle += stCycle;
    }


}