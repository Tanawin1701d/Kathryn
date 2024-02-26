//
// Created by tanawin on 30/11/2566.
//

#include "value.h"

#include <utility>
#include "model/controller/controller.h"
#include "util/numberic/numConvert.h"


namespace kathryn{

    void Val::com_init() {
        ctrl->on_value_init(this);
    }

    SliceAgent<Val>& Val::operator()(int start, int stop){

        auto ret = new SliceAgent<Val>(
                this,
                getAbsSubSlice(start, stop, getSlice())
                );
        return *ret;
    }

    SliceAgent<Val>& Val::operator() (int idx){
        return operator() (idx, idx+1);
    }


    Operable *Val::checkShortCircuit() {
        return nullptr;
    }



    /**
     * value simulation
     * */

    ValLogicSim::ValLogicSim(Val* master,
                             int sz,
                             VCD_SIG_TYPE sigType,
                             bool simForNext):
            LogicSimEngine(sz, sigType, simForNext),
            _master(master){}

    void ValLogicSim::simStartCurCycle() {
        /** val don't have to simulate*/
    }

    void ValLogicSim::simExitCurCycle() {
        /** we do this to prevent iterate() function to prepare for next cycle*/
    }

    void ValLogicSim::initSim(){
        setCurValSimStatus();
        setNextValSimStatus();
        getCurVal()  = _master->rawValue;
        getNextVal() = _master->rawValue;
    }



}