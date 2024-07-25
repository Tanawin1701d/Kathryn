//
// Created by tanawin on 18/7/2024.
//

#include "valueSim.h"
#include "model/hwComponent/value/value.h"
#include "sim/modelSimEngine/hwComponent/abstract/genHelper.h"


namespace kathryn{


    /**
         * value simulation
         * */

    ValSimEngine::ValSimEngine(Val* master,
                             VCD_SIG_TYPE sigType,
                             ull rawValue):
    LogicSimEngine(master, master,
                   VST_INTEGER, false, rawValue),
    _master(master){ assert(master != nullptr);}


    ValR ValSimEngine::genSrcOpr(){
        int size = _asb->getAssignSlice().getSize();

        ValR x(getValR_Type(), size,
            std::to_string(_initVal)+CXX_ULL_SUFFIX);
        return x.cast(getValR_Type(), size);
    }

    void
    ValSimEngine::createGlobalVariable(CbBaseCxx& cb){}


}
