//
// Created by tanawin on 1/6/2024.
//

#include "proxyEventBase.h"
#include "params/simParamType.h"


#include <utility>

namespace kathryn{
    ProxySimEventBase::ProxySimEventBase():
        EventBase(0, SIM_MODEL_PRIO, false),
        VCD_REC_POL(MDE_REC_SKIP),
        _vcdWriter(nullptr){
#ifdef MODELCOMPILEVB
    std::cout << "constructor of proxy sim event base" << std::endl;
#endif
    }

    ProxySimEventBase::~ProxySimEventBase(){ delete _vcdWriter; }

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

    void ProxySimEventBase::simStartLongRunCycle(){
        assert(_isLongRangeSim);
        std::cout << "start long range run with amount lim cycle "<<
            _amtLimitLongRangeCycle << std::endl;
        amtLRSim = mainSim();

        //////std::cout << "long range get  amtLRSim "<< amtLRSim << std::endl;
    }

    /*
     *  single cycle simulation
     */

    void ProxySimEventBase::simStartCurCycleNeg(){
        if (isLongRageSim()){return;}
        startMainOpEleSimNeg();
    }

    void ProxySimEventBase::simStartCurCyclePos(){
        if (isLongRageSim()){return;}
        startMainOpEleSimPos();
    }

    void ProxySimEventBase::curCycleCollectDataNeg(){

        if (isLongRageSim()){return;}
        writeVcdSignal();
    }
    void ProxySimEventBase::curCycleCollectDataPos(){
        if (isLongRageSim()){return;}
        writeVcdSignal();
        startPerfCol();
    }

    void ProxySimEventBase::simStartNextCycleNeg(){
        if (isLongRageSim()){return;}
        startFinalizeEleSimNeg();
        //////// sim register change exact register
           ///////////// do not wory about register simulation will get false new
           ///data from memory if there is update from memory to register because
           /// memEleHolder will provide temporary data to register simulation
    }

    void ProxySimEventBase::simStartNextCyclePos(){
        if (isLongRageSim()){return;}
        startFinalizeEleSimPos();
    }

    EventBase* ProxySimEventBase::genNextEvent(){
        ///////////////std::cout << _targetCycle << std::endl;
        if (isLongRageSim()){
            _targetCycle += getAmtLRsim();
        }else{
            _targetCycle++;
        }
        return this;
    }

    void ProxySimEventBase::writeVcdSignal(){

        if ((VCD_REC_POL == MDE_REC_BOTH) |
            (VCD_REC_POL == MDE_REC_ONLY_USER) |
            (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
        ){
            assert(_vcdWriter !=nullptr);

            if (clkSignal == 1){ ///// the old value is 1
                _vcdWriter->addNewTimeStamp(getCurCycle() * 10 + 5);
                clkSignal = 0;
            }else{ ///// the old value is 0
                ///// the new value is 1
                _vcdWriter->addNewTimeStamp(getCurCycle() * 10);
                clkSignal = 1;
            }
            _vcdWriter->addNewValue(CLK_SIGNAL, clkSignal);

        }

        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_USER)){
            //std::cout << "add vcd user col" << std::endl;
            startVcdColUser();
            //std::cout << "add vcd fin" << std::endl;
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
            startVcdColInternal();
        }

        // if ((VCD_REC_POL == MDE_REC_BOTH) |
        //     (VCD_REC_POL == MDE_REC_ONLY_USER) |
        //     (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
        // ){
        //     _vcdWriter->addNewTimeStamp(getCurCycle() * 10 + 5);
        //     clkSignal = 0;
        //     _vcdWriter->addNewValue(CLK_SIGNAL, clkSignal);
        // }

    }

}
