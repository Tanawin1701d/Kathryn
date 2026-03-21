//
// Created by tanawin on 18/7/2024.
//

#include "value_sim.h"
#include "model/hw_component/value/value.h"
#include "sim/model_sim_engine/hw_component/abstract/gen_helper.h"


namespace kathryn{


    /**
         * value simulation
         * */

    ValSimEngine::ValSimEngine(Val* master,
                             VCD_SIG_TYPE sig_type,
                             ull raw_value):
    LogicSimEngine(master, master, master,
                   VST_INTEGER, false, raw_value),
    _master(master){ assert(master != nullptr);}


    ValR ValSimEngine::get_val_rep(){
        int size = _asb->get_assign_slice().get_size();

        ValR x(getValR_Type(), size,
            std::to_string(_initVal)+CXX_ULL_SUFFIX);
        /////// due to ull string init we must cast first
        return x.cast_base(x._valType, x._size);
    }

    void
    ValSimEngine::create_global_variable(CbBaseCxx& cb){}


}
