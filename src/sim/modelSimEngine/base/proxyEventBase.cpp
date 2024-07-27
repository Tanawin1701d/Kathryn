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

    void ProxySimEventBase::simStartCurCycle(){
        ///// the order is very strict
        ///do not change to simulation order
        ///std::cout << "startSimCur maybe" << std::endl;
        if (isLongRageSim()){
            ////std::cout << "skip cause long range run" << std::endl;
            return;
        }
        ////std::cout << "startSimCur" << std::endl;
        startMainOpEleSim(); ////// wire mem eleHolder nest expression
    }

    void ProxySimEventBase::curCycleCollectData(){
        ///// start collect vcd
        ///
        //std::cout << "ddddd" << std::endl;
        if (isLongRageSim()){
            ////std::cout << "skip cause long range run" << std::endl;
            return;
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) |
            (VCD_REC_POL == MDE_REC_ONLY_USER) |
            (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
        ){
            //std::cout << "add Time Stamp" << std::endl;
            assert(_vcdWriter !=nullptr);
            //std::cout << _vcdWriter << std::endl;
            _vcdWriter->addNewTimeStamp(getCurCycle() * 10);
            clkSignal = 1;
            _vcdWriter->addNewValue(CLK_SIGNAL, clkSignal);
            //std::cout << "add Time Stamp fin" << std::endl;
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_USER)){
            //std::cout << "add vcd user col" << std::endl;
            startVcdColUser();
            //std::cout << "add vcd fin" << std::endl;
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) | (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)){
            startVcdColInternal();
        }

        if ((VCD_REC_POL == MDE_REC_BOTH) |
            (VCD_REC_POL == MDE_REC_ONLY_USER) |
            (VCD_REC_POL == MDE_REC_ONLY_INTERNAL)
        ){
            _vcdWriter->addNewTimeStamp(getCurCycle() * 10 + 5);
            clkSignal = 0;
            _vcdWriter->addNewValue(CLK_SIGNAL, clkSignal);
        }

        /////// start collect per collection
        //std::cout << "before start perf" << std::endl;
        startPerfCol();
    }

    void ProxySimEventBase::simStartNextCycle(){
        if (isLongRageSim()){
            /////std::cout << "skip cause long range run" << std::endl;
            return;
        }
        startFinalizeEleSim();
        //////// sim register change exact register
           ///////////// do not wory about register simulation will get false new
           ///data from memory if there is update from memory to register because
           /// memEleHolder will provide temporary data to register simulation
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
}
