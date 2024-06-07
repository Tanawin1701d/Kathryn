//
// Created by tanawin on 1/6/2024.
//

#include "proxyEventBase.h"
#include "params/simParamType.h"


#include <utility>

namespace kathryn{


ProxySimEventBase::ProxySimEventBase():
    EventBase     (0,SIM_MODEL_PRIO),
    _VCD_REC_POL  (MDE_REC_SKIP),
    _vcdWriter    (nullptr){
    std::cout << "constructor of proxy sim event base" << std::endl;
}

ProxySimEventBase::~ProxySimEventBase(){delete _vcdWriter;}

void ProxySimEventBase::eventWarmUp(){

    startRegisterCallBack();
    if ((_VCD_REC_POL == MDE_REC_BOTH) | (_VCD_REC_POL == MDE_REC_ONLY_USER)){
        startVcdDecVarUser();
    }
    if ((_VCD_REC_POL == MDE_REC_BOTH) | (_VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
        startVcdDecVarInternal();
    }
}

void ProxySimEventBase::simStartCurCycle(){
    ///// the order is very strict
    ///do not change to simulation order
    startVolatileEleSim(); ////// wire mem eleHolder nest expression
}

void ProxySimEventBase::curCycleCollectData(){
    ///// start collect vcd

        if ((_VCD_REC_POL == MDE_REC_BOTH) |
            (_VCD_REC_POL == MDE_REC_ONLY_USER) |
            (_VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
            ){
            _vcdWriter->addNewTimeStamp(getCurCycle()*10);
        }

        if ((_VCD_REC_POL == MDE_REC_BOTH) | (_VCD_REC_POL == MDE_REC_ONLY_USER)){
            startVcdColUser();
        }

        if ((_VCD_REC_POL == MDE_REC_BOTH) | (_VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
            startVcdColInternal();
        }

        /////// start collect per collection
        startPerfCol();
}

void ProxySimEventBase::simStartNextCycle(){
    startNonVolatileEleSim(); //////// sim register change exact register
    ///////////// do not wory about register simulation will get false new
    ///data from memory if there is update from memory to register because
    /// memEleHolder will provide temporary data to register simulation
}

EventBase* ProxySimEventBase::genNextEvent(){
    _targetCycle++;
    return this;
}



}
