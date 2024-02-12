//
// Created by tanawin on 25/1/2567.
//

#include "simInterface.h"

#include <utility>



namespace kathryn{

    /**
     *
     * sim interface
     *
     * */



    SimInterface::SimInterface(CYCLE limitCycle,
                               std::string vcdFilePath,
                               std::string profileFilePath):
            _vcdWriter(new VcdWriter(std::move(vcdFilePath))),
            _flowWriter(new FlowWriter(std::move(profileFilePath))),
            _limitCycle(limitCycle)
    {
        _ModuleSimEvent = new ModuleSimEvent(getGlobalModulePtr(),
                                             rstWire,
                                             _vcdWriter,
                                             _flowWriter->getstartEle()
                                             );
        SimController* simCtrl = getSimController();
        assert(simCtrl != nullptr);
        simCtrl->setLimitCycle(limitCycle);
    }

    SimInterface::~SimInterface() {
        delete _vcdWriter;
        delete _flowWriter;
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