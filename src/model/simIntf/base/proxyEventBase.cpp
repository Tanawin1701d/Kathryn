//
// Created by tanawin on 1/6/2024.
//

#include "proxyEventBase.h"
#include "params/simParamType.h"


#include <utility>

namespace kathryn{


ProxySimEventBase::ProxySimEventBase():
    EventBase     (0,SIM_MODEL_PRIO),
    VCD_REC_POL  (MDE_REC_SKIP),
    _vcdWriter    (nullptr){
#ifdef MODELCOMPILEVB
    std::cout << "constructor of proxy sim event base" << std::endl;
#endif
}

ProxySimEventBase::~ProxySimEventBase(){delete _vcdWriter;}

void ProxySimEventBase::eventWarmUp(){

    startRegisterCallBack();
    _vcdWriter->addNewVar(VST_REG, CLK_SIGNAL, {0, 1});
    if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_USER)){
        startVcdDecVarUser();
    }
    if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
        startVcdDecVarInternal();
    }
}

void ProxySimEventBase::simStartCurCycle(){
    ///// the order is very strict
    ///do not change to simulation order
    startMainOpEleSim(); ////// wire mem eleHolder nest expression
}

void ProxySimEventBase::curCycleCollectData(){
    ///// start collect vcd

        if ((VCD_REC_POL == MDE_REC_BOTH) |
            (VCD_REC_POL == MDE_REC_ONLY_USER) |
            (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
            ){

            _vcdWriter->addNewTimeStamp(getCurCycle()*10);
            clkSignal = 1;
            _vcdWriter->addNewValue(CLK_SIGNAL, &clkSignal);
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_USER)){
            startVcdColUser();
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
            startVcdColInternal();
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) |
                (VCD_REC_POL == MDE_REC_ONLY_USER) |
                (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
                ){
            _vcdWriter->addNewTimeStamp(getCurCycle()*10 + 5);
            clkSignal = 0;
            _vcdWriter->addNewValue(CLK_SIGNAL, &clkSignal);
        }

        /////// start collect per collection
        startPerfCol();
}

void ProxySimEventBase::simStartNextCycle(){
    startFinalizeEleSim(); //////// sim register change exact register
    ///////////// do not wory about register simulation will get false new
    ///data from memory if there is update from memory to register because
    /// memEleHolder will provide temporary data to register simulation
}

EventBase* ProxySimEventBase::genNextEvent(){
    ///////////////std::cout << _targetCycle << std::endl;
    _targetCycle++;
    return this;
}



}
