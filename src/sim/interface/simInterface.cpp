//
// Created by tanawin on 25/1/2567.
//

#include <iostream>

#include <utility>

#include "simInterface.h"
#include "model/simIntf/base/proxyBuildMng.h"


namespace kathryn{

    /**
     *
     * sim interface
     *
     * */



    SimInterface::SimInterface(CYCLE limitCycle,
                               std::string vcdFilePath,
                               std::string profileFilePath,
                               std::string clientSimPath = "error"
                               ):
            SIM_CLIENT_PATH(std::move(clientSimPath)),
            _vcdWriter (new VcdWriter(std::move(vcdFilePath))),
            _flowWriter(new FlowWriter(std::move(profileFilePath))),
            _limitCycle(limitCycle)
    {
        SimController* simCtrl = getSimController();
        assert(simCtrl != nullptr);
        simCtrl->setLimitCycle(limitCycle);
    }

    SimInterface::~SimInterface() {
        delete _vcdWriter;
        delete _flowWriter;
        delete _modelSimEvent;
        /**no need to delete user event because sim controller will delete it */
    }

    void SimInterface::simStart() {
        /** set assiging mode*/
        setAssignMode(AM_SIM);
        /***compile and link module sim Event */
        createModelSimEvent();
        /***con simulating*/
        describe();
        /**con simulating*/
        simStartConSim();
        /** start main thread*/
        SimController* simCtrl = getSimController();
        assert(simCtrl != nullptr);
        simCtrl->start();
        /** sim ctrl now finish next terminate our sim specifier*/
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
        conThread = std::make_unique<std::thread>(&SimInterface::describeConWrapper,this);
        assert(conThread != nullptr);
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

    void SimInterface::conNextCycle(CYCLE amtCycle) {
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

    /***
     *
     * test value
     *
     */

    void SimInterface::testAndPrint(const std::string& testName, ValRepBase& simValLhs, ValRepBase& testValRhs) {

        if (simValLhs.getVal() == testValRhs.getVal()){
            std::cout << TC_GREEN << testName << " pass " << TC_DEF << std::endl;
        }else{
            std::cout << TC_RED << testName << " fail expect: "
                      << testValRhs.getBiStr() << "  got : "
                      << simValLhs.getBiStr() << TC_DEF << std::endl;
        }
    }

    void SimInterface::testAndPrint(const std::string& testName, ull simVal, ull expect) {

        if (simVal == expect) {
            std::cout << TC_GREEN << testName << " pass " << TC_DEF << std::endl;
        } else {
            std::cout << TC_RED << testName << " fail expect: "
                      << expect << "  got : "
                      << simVal << TC_DEF << std::endl;
        }

    }


    /**
     *
     * create model client
     *
     */

    void SimInterface::createModelSimEvent(){

        /** generate c++ file**/
        ProxyBuildMng buildMng(getGlobalModulePtr()); /// todo , SIM_CLIENT_PATH);
        ///buildMng.startWriteModelSim();
        buildMng.startCompile();
        _modelSimEvent = buildMng.loadAndGetProxy();
        _modelSimEvent->setVcdWriter(_vcdWriter);



    }






}
