//
// Created by tanawin on 14/1/2025.
//

#include "pm_value_sim.h"

#include "model/hw_component/value/pm_val.h"

namespace kathryn{

    PmValSimEngine::PmValSimEngine(
        PmVal* master,
        VCD_SIG_TYPE sig_type
    ):
    LogicSimEngine(master, master, master,
        VST_INTEGER, false,
        master->_rawValue),
    _master(master){assert(master != nullptr);}

    ValR PmValSimEngine::get_val_rep(){
        int size = _asb->get_assign_slice().get_size();

        ValR x(getValR_Type(), size,
            std::to_string(_initVal)+CXX_ULL_SUFFIX);
        /////// due to ull string init we must cast first
        return x.cast_base(x._valType, x._size);
    }

}
