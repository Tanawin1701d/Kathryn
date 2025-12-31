//
// Created by tanawin on 14/1/2025.
//

#include "pmValueSim.h"

#include "model/hwComponent/value/PmVal.h"

namespace kathryn{

    PmValSimEngine::PmValSimEngine(
        PmVal* master,
        VCD_SIG_TYPE sigType
    ):
    LogicSimEngine(master, master, master,
        VST_INTEGER, false,
        master->_rawValue),
    _master(master){assert(master != nullptr);}

    ValR PmValSimEngine::getValRep(){
        int size = _asb->getAssignSlice().getSize();

        ValR x(getValR_Type(), size,
            std::to_string(_initVal)+CXX_ULL_SUFFIX);
        /////// due to ull string init we must cast first
        return x.castBase(x._valType, x._size);
    }

}
