//
// Created by tanawin on 25/1/2567.
//

#include <iostream>

#include <utility>

#include "simInterface.h"

#include "params/simParam.h"
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
                               std::string clientSimPath
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
        ////delete _modelSimEvent;
        /**no need to delete user event because sim controller will delete it */
    }

    void SimInterface::simStart() {
        /** set assiging mode*/
        setAssignMode(AM_SIM);
        /***compile and link module sim Event */
        createModelSimEvent();
        /***con simulating*/
        describeDef();
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


    void SimInterface::describeDef(){
        ////*rstWire = 1;
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

    void SimInterface::conCycleBase(CYCLE startCycle){

        lastCtTrigger->finishSimCurEvent.notify(conCurCycleUsed);
        lastCtTrigger->startEndCycleEvent.wait(conCurCycleUsed);
        lastCtTrigger->setFutureCycle(startCycle);
        lastCtTrigger->finishEndCycleEvent.notify(conCurCycleUsed);
        lastCtTrigger->startSimCurEvent.wait(startCycle);
        conCurCycleUsed = startCycle;


    }

    void SimInterface::conCycle(CYCLE startCycle){
        conCycleBase(startCycle);
    }

    void SimInterface::conNextCycle(CYCLE amtCycle) {
        conCycle(conCurCycleUsed + amtCycle);
    }

    void SimInterface::conEndCycle() {
        lastCtTrigger->finishSimCurEvent.notify(conCurCycleUsed);
        lastCtTrigger->startEndCycleEvent.wait(conCurCycleUsed);
    }


    void SimInterface::describeConWrapper() {
        assert(lastCtTrigger != nullptr);
        lastCtTrigger->startSimCurEvent.wait(2);
        describeCon();
        ///////std::cout << "last trigger notifying" << std::endl;
        lastCtTrigger->finishSimCurEvent.notify(conCurCycleUsed);
        lastCtTrigger->markStop();
        lastCtTrigger->finishEndCycleEvent.notify(conCurCycleUsed);
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
        _proxyBuildMng.setStartModule(getGlobalModulePtr()); /// todo , SIM_CLIENT_PATH);
        ///buildMng.startWriteModelSim();
        _proxyBuildMng.startCompile();
        _modelSimEvent = _proxyBuildMng.loadAndGetProxy();
        _modelSimEvent->setVcdWriter(_vcdWriter);
        _modelSimEvent->setVcdWritePol(PARAM_VCD_REC_POL);
        getSimController()->addEvent(_modelSimEvent);



    }






}
