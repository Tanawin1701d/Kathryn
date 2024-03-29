//
// Created by tanawin on 29/3/2567.
//

#ifndef KATHRYN_MODULESIMENGINE_H
#define KATHRYN_MODULESIMENGINE_H

#include "model/simIntf/base/modelSimEngine.h"

namespace kathryn{



    /***
     * module interface
     * */
    class ModuleSimEngine : public SimEngine{

    protected:
        bool isCurCycleSimulated  = false;
        bool isNextCycleSimulated = false;
    public:
        explicit ModuleSimEngine():
                SimEngine(){}

        virtual void beforePrepareSim(VcdWriter* vcdWriter, FlowColEle* flowColEle) = 0;

        void setCurValSimStatus () override{isCurCycleSimulated = true;}
        void setNextValSimStatus() override{isNextCycleSimulated = true;}

        bool     isCurValSim () const override{return isCurCycleSimulated; }
        bool     isNextValSim() const override{return isNextCycleSimulated;}

        ValRep&  getCurVal () override {assert(false);}
        ValRep&  getNextVal() override {assert(false);}

    };


}

#endif //KATHRYN_MODULESIMENGINE_H
