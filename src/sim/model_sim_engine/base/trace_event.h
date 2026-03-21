//
// Created by tanawin on 20/7/2024.
//

#ifndef TRACEEVENT_H
#define TRACEEVENT_H
#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"

#include "model/hw_component/abstract/operable.h"
#include "sim/model_sim_engine/hw_component/abstract/gen_helper.h"

namespace kathryn{


    struct TraceEvent{
        Operable* _condOpr = nullptr;
        std::function<void(void)> _callback;

        TraceEvent(Operable& opr1, std::function<void(void)> callback):
        _condOpr(&opr1),
        _callback(callback){
            assert(opr1.get_operable_slice().get_size() == 1);
             LogicSimEngine* lse = opr1.get_logic_sim_engine_from_opr_ptr();
            assert(lse != nullptr);
            lse->req_glob_dec();
        }

        [[nodiscard]] std::string get_cond_str() const{
            return get_sliced_src_opr_from_opr(_condOpr).to_string();
        }

        void exec_call_back() const{
            _callback();
        }

    };





}

#endif //TRACEEVENT_H
