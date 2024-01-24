//
// Created by tanawin on 30/11/2566.
//

#include "value.h"

#include <utility>
#include "model/controller/controller.h"
#include "util/numberic/numConvert.h"


namespace kathryn{

    Val::Val(int size, std::string v):
            LogicComp({0, size}, TYPE_VAL,false),
            rawValue(std::move(v)),
            _size(size){
                    com_init();
                    ValRep assignVal = cvStrToValRep(rawValue);
                    getSimEngine()->getCurVal()  = assignVal;
                    getSimEngine()->getBackVal() = assignVal;

            }

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
        resetSimStatus();
        //// getSimEngine()->iterate();
        //////// don't have to iterate due to it is fix value
    }


}