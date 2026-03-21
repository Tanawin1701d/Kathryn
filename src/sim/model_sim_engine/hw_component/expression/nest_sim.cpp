//
// Created by tanawin on 18/7/2024.
//

#include "nest_sim.h"
#include "model/hw_component/expression/nest.h"

namespace kathryn{


    /***
     *
     * NestLogicSim
     *
     * */

    NestSimEngine::NestSimEngine(nest* master,
                             VCD_SIG_TYPE sig_type):
            LogicSimEngine(master, master, master, sig_type, false, 0),
            _master(master){
        assert(_master != nullptr);
    }

    void NestSimEngine::proxy_build_init(){
        for (NestMeta& meta: _master->_nestList){
            dep.push_back(meta.opr1->get_logic_sim_engine_from_opr_ptr());
        }
    }


    void NestSimEngine::create_op(CbBaseCxx& cb){
        ///////// build string
        cb.add_cm(_ident->get_global_name());

        /////////// we build from low priority to high priority
        int start_idx = 0;

        for (NestMeta& meta: _master->_nestList){
            //////// data preparation
            Operable* opr1   = meta.opr1;
            Assignable* asb = meta.asb;
            assert(opr1 != nullptr);
            if (!_master->read_only){
                assert(asb != nullptr);
            }
            int cur_size = opr1->get_operable_slice().get_size();
            cb.add_st(gen_assign_a_eq_b({start_idx, start_idx + cur_size}, false, opr1));
            start_idx += cur_size;
        }
        assert(start_idx == _asb->get_assign_slice().get_size());
    }

}