//
// Created by tanawin on 25/1/2567.
//

#include "simInterface.h"



namespace kathryn{

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
        delete _vcdWriter;
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

}