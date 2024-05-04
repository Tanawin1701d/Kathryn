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
        simCtrl->start();
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
        lastCtTrigger = new ConcreteTriggerEvent(2,this,
                                                 [&](){return true;}, SIM_CC_TRIGGER_PRIO_FRONT_CYCLE);
        conThread = std::make_unique<std::thread>(&SimInterface::describeConWrapper, this);
        assert(conThread != nullptr);
        /** to prevent simulation queue dead lock*/
    }

    void SimInterface::conCycleBase(CYCLE startCycle, int priority){

        /////// create new event and push to the queue first to prevent old trigger release
        /////// their event and let event queue iterate over our section
        conCurCycleUsed = startCycle;
        auto newCtTrigger = new ConcreteTriggerEvent(startCycle,
                                                 this,
                                                 [](){return true;},
                                                 priority);
        /** new we ensure that next event is schedule now release old event****/
        if (lastCtTrigger != nullptr){
            lastCtTrigger->getFinishSerializer().notify();
            /** the trigger will be automatically delete from sim controller*/
        }


        lastCtTrigger = newCtTrigger;
        lastCtTrigger->getStartSerializer().wait();
    }

    void SimInterface::conCycle(CYCLE startCycle){
        conCycleBase(startCycle, SIM_CC_TRIGGER_PRIO_FRONT_CYCLE);
    }

    void SimInterface::conNextCycle(kathryn::CYCLE amtCycle) {
        conCycle(conCurCycleUsed + amtCycle);
    }

    void SimInterface::conEndCycle() {
        conCycleBase(conCurCycleUsed, SIM_CC_TRIGGER_PRIO_BACK_CYCLE);
    }


    void SimInterface::describeConWrapper() {
        assert(lastCtTrigger != nullptr);
        lastCtTrigger->getStartSerializer().wait();
        describeCon();
        ///////std::cout << "last trigger notifying" << std::endl;
        lastCtTrigger->getFinishSerializer().notify();
        /** to prevent queue stuck*/
    }

    void SimInterface::testAndPrint(std::string testName, ValRep &simValLhs, ValRep& testValRhs) {

        if ((simValLhs == testValRhs).getLogicalValue()){
            std::cout << TC_GREEN << testName << " pass " << TC_DEF << std::endl;
        }else{
            std::cout << TC_RED << testName << " fail expect: "
                      << testValRhs.getBiStr() << "  got : "
                      << simValLhs.getBiStr() << TC_DEF << std::endl;
        }
    }

    void SimInterface::testAndPrint(std::string testName, ull simVal, ull expect) {

        if (simVal == expect) {
            std::cout << TC_GREEN << testName << " pass " << TC_DEF << std::endl;
        } else {
            std::cout << TC_RED << testName << " fail expect: "
                      << expect << "  got : "
                      << simVal << TC_DEF << std::endl;
        }

    }



}