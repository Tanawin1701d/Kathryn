//
// Created by tanawin on 1/6/2024.
//

#include "proxyEventBase.h"


#include <utility>
#include "params/simParam.h"


namespace kathryn{


ProxySimEventBase::ProxySimEventBase():
    EventBase     (0,SIM_MODEL_PRIO),
    _vcdWriter    (nullptr){}

ProxySimEventBase::~ProxySimEventBase(){delete _vcdWriter;}

void ProxySimEventBase::eventWarmUp(){

    startRegisterCallBack();
    if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) | (PARAM_VCD_REC_POL == MDE_REC_ONLY_USER)){
        startVcdDecVarUser();
    }
    if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) | (PARAM_VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
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

        if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) |
            (PARAM_VCD_REC_POL == MDE_REC_ONLY_USER) |
            (PARAM_VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
            ){
            _vcdWriter->addNewTimeStamp(getCurCycle()*10);
        }

        if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) | (PARAM_VCD_REC_POL == MDE_REC_ONLY_USER)){
            startVcdColUser();
        }

        if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) | (PARAM_VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
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
