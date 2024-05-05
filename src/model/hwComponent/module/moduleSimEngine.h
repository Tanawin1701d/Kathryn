//
// Created by tanawin on 29/3/2567.
//

#ifndef KATHRYN_MODULESIMENGINE_H
#define KATHRYN_MODULESIMENGINE_H

#include "params/simParam.h"
#include "model/simIntf/base/modelSimEngine.h"

namespace kathryn{



    /***
     * module interface
     * */
    class ModuleSimEngine : public SimEngine{

    protected:
        MODULE_VCD_REC_POL _mdRecPol;
        bool isCurCycleSimulated  = false;
        bool isNextCycleSimulated = false;
    public:
        explicit ModuleSimEngine():
                _mdRecPol(PARAM_VCD_REC_POL),
                SimEngine(){}

        virtual void beforePrepareSim(VcdWriter* vcdWriter, FlowColEle* flowColEle) = 0;

        void setCurValSimStatus () override{isCurCycleSimulated = true;}
        void setNextValSimStatus() override{isNextCycleSimulated = true;}

        void setVcdRegPol(MODULE_VCD_REC_POL recPol){_mdRecPol = recPol;}

        bool     isCurValSim () const override{return isCurCycleSimulated; }
        bool     isNextValSim() const override{return isNextCycleSimulated;}

        ValRep&  getCurVal () override {assert(false);}
        ValRep&  getNextVal() override {assert(false);}

    };


}

#endif //KATHRYN_MODULESIMENGINE_H
