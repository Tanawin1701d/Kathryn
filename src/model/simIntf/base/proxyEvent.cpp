//
// Created by tanawin on 1/6/2024.
//

#include "proxyEvent.h"


namespace kathryn{


    ProxySimEvent::ProxySimEvent(std::string vcdWriteDes):
        EventBase(0,SIM_MODEL_PRIO),
        VCD_WRITE_DES(vcdWriteDes),
        vcdWriter(new VcdWriter(vcdWriteDes)){


        if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) | (PARAM_VCD_REC_POL == MDE_REC_ONLY_USER)){
            startVcdDecVarUser();
        }

        if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) | (PARAM_VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
            startVcdDecVarInternal();
        }

    }

    ProxySimEvent::~ProxySimEvent(){
        delete vcdWriter;
    }



void ProxySimEvent::simStartCurCycle(){

    ///// the order is very strict
    ///do not change to simulation order
    startVolatileEleSim(); ////// wire mem eleHolder nest expression
}

void ProxySimEvent::curCycleCollectData(){
    ///// start collect vcd
        if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) | (PARAM_VCD_REC_POL == MDE_REC_ONLY_USER)){
            startVcdColUser();
        }

        if ((PARAM_VCD_REC_POL == MDE_REC_BOTH) | (PARAM_VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
            startVcdColInternal();
        }

        /////// start collect per collection
        startPerfCol();
}

void ProxySimEvent::simStartNextCycle(){
    startNonVolatileEleSim(); //////// sim register change exact register
    ///////////// do not wory about register simulation will get false new
    ///data from memory if there is update from memory to register because
    /// memEleHolder will provide temporary data to register simulation
}









}
