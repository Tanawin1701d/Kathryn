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

    SliceAgent<Val>& Val::operator() (Slice sl){
        return operator() (sl.start, sl.stop);
    }


    Operable* Val::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }


    Operable* Val::checkShortCircuit() {
        return nullptr;
    }



    /**
     * value simulation
     * */

    ValSimEngine::ValSimEngine(Val* master,
                             VCD_SIG_TYPE sigType):
    LogicSimEngine(master, master,
                   VST_INTEGER, false, master->_rawValue),
    _master(master){ assert(master != nullptr);}



}