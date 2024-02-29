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
        /** set assiging mode*/
        setAssignMode(AM_SIM);
        /***module sim Event is auto insert to event*/

        /***con simulating*/
        describe();
        /**con simulating*/
        simStartConSim();
        /** start main thread*/
        SimController* simCtrl = getSimController();
        assert(simCtrl != nullptr);
        simCtrl->simStart();
        if (conThread && conThread->joinable()){
            conThread->join();
        }
        /** set assiging mode back*/
        setAssignMode(AM_MOD);
        /** finish*/

    }


    /***
     *
     *
     * Concrete simulation
     *
     * */


    void SimInterface::simStartConSim(){
        /**build new auto start trigger*/
        lastCtTrigger = new ConcreteTriggerEvent(2,this, [&](){return true;});
        conThread = std::make_unique<std::thread>(&SimInterface::describeConWrapper, this);
        assert(conThread != nullptr);
        /** to prevent simulation queue dead lock*/
    }

    void SimInterface::conCycle(CYCLE startCycle){
        /** move on old trigger first*/
        if (lastCtTrigger != nullptr){
            lastCtTrigger->getFinishSerializer().notify();
            /** the trigger will be automatically delete from sim controller*/
        }


        conCurCycleUsed = startCycle;
        lastCtTrigger = new ConcreteTriggerEvent(startCycle,
                                                 this,
                                                 [](){return true;});
        lastCtTrigger->getStartSerializer().wait();
    }

    void SimInterface::conNextCycle(kathryn::CYCLE amtCycle) {
        conCycle(conCurCycleUsed + amtCycle);
    }

    void SimInterface::describeConWrapper() {
        assert(lastCtTrigger != nullptr);
        lastCtTrigger->getStartSerializer().wait();
        describeCon();
        lastCtTrigger->getFinishSerializer().notify();
        /** to prevent queue stuck*/
    }





}