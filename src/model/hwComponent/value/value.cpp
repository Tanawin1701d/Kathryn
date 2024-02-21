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

    void Val::simStartCurCycle() {
        /** val don't have to simulate*/
    }

    void Val::simExitCurCycle() {
        /** we do this to prevent iterate() function to prepare for next cycle*/
    }

    void Val::initSim(){
        getRtlValItf()->setCurValSimStatus();
        getRtlValItf()->setNextValSimStatus();
        getRtlValItf()->getCurVal()  = rawValue;
        getRtlValItf()->getNextVal() = rawValue;
    }

    Operable *Val::checkShortCircuit() {
        return nullptr;
    }


}