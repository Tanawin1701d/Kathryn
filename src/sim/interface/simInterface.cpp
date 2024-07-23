//
// Created by tanawin on 25/1/2567.
//

#include <iostream>

#include <utility>

#include "simInterface.h"
#include "params/simParam.h"


namespace kathryn{

    /**
     *
     * sim interface
     *
     * */



    SimInterface::SimInterface(CYCLE limitCycle,
                               std::string vcdFilePath,
                               std::string profileFilePath,
                               std::string genFileName
                               ):
            _vcdWriter (new VcdWriter(std::move(vcdFilePath))),
            _flowWriter(new FlowWriter(std::move(profileFilePath))),
            _limitCycle(limitCycle),
            _proxyBuildMng(std::move(genFileName)),
            requireConSim(true)
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
        describeModelTriggerWrapper();
        createModelSimEvent();
        /***con simulating*/
        describeDef();
        describe();
        /**con simulating*/
        if (requireConSim){
            simStartConSim();
        }
        /** start main thread*/
        SimController* simCtrl = getSimController();
        assert(simCtrl != nullptr);
        simCtrl->setTriggerMap(&_traceEvents);
        simCtrl->setLrLimUser(&_nextLimitAmtLRC);
        simCtrl->start();
        /** sim ctrl now finish next terminate our sim specifier*/
        if (requireConSim && conThread && conThread->joinable()){
            conThread->join();
        }
        /** set assiging mode back*/
        setAssignMode(AM_MOD);
        /** finish*/

    }


    void SimInterface::describeDef(){
        sim{
            *rstWire = 1;
        };
        incCycle(1);
        sim{
            *rstWire = 0;
        };

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


    /**
     *
     * trigger manager
     *
     ***/

    void SimInterface::setNextLimitAmtLRC(CYCLE amtCycle){
        assert(amtCycle > 0);
        _nextLimitAmtLRC = amtCycle;
    }

    void SimInterface::trig(Operable& opr, std::function<void()> callback){
        _traceEvents.emplace_back(opr, callback);
    }

    void SimInterface::trig(Operable& opr, SIM_INTERFACE_EVENT event){
        switch (event){
            case EXIT_SIM:{
                _traceEvents.emplace_back(opr, [](){
                    getSimController()->stopSim();
                });
            }
            default : {assert(false);}
        }
        ///// TODO make the system exit  and

    }

    void SimInterface::describeModelTriggerWrapper(){
        getControllerPtr()->on_globalModule_init_auxilaryComponent();
        describeModelTrigger();
        getControllerPtr()->on_globalModule_final_auxilaryComponent();
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
                      << std::to_string(testValRhs.getVal()) << "  got : "
                      << std::to_string(simValLhs.getVal())  << TC_DEF << std::endl;
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
        _proxyBuildMng.setTracer(&_traceEvents);
        _proxyBuildMng.startWriteModelSim();
        _proxyBuildMng.startCompile();
        _modelSimEvent = _proxyBuildMng.loadAndGetProxy();

        ///////// initialize both simevent and proxyBuildMng
        _modelSimEvent->setVcdWriter(_vcdWriter);
        _modelSimEvent->setVcdWritePol(PARAM_VCD_REC_POL);
        _modelSimEvent->eventWarmUp();
        _proxyBuildMng.startRetrieveSimVal(_modelSimEvent);

        ////////// add to event queue
        getSimController()->addEvent(_modelSimEvent);



    }






}
