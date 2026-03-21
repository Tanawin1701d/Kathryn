//
// Created by tanawin on 11/12/2566.
//

#include "state_reg_util.h"


/***
 *
 * lagacy
 *
 * */

namespace kathryn{


    std::vector<StResMeta> get_state_info(StateReg* st_reg_ptr){

        // std::vector<StResMeta> pre_result;
        // auto& update_metas = st_reg_ptr->get_update_meta();
        //
        // for (auto up_event : update_metas){
        //     /////// get update event for that state register
        //     StResMeta result;
        //     result.condition = up_event->src_update_condition;
        //     /** recursively get state as */
        //     result.depend_state = get_depend_state(up_event->src_update_state);
        //     pre_result.push_back(result);
        // }
        //
        // return pre_result;
        return {};
    }



    std::vector<Operable*> get_depend_state(Operable* depend_opr){
        if (depend_opr == nullptr){
            return {};
        }
        Identifiable* ident = depend_opr->cast_to_ident();
        HW_COMPONENT_TYPE hw_type = ident->get_type();

        switch (hw_type){

            case TYPE_REG:
            case TYPE_STATE_REG:
            case TYPE_WIRE:
            case TYPE_VAL:
                return {depend_opr};
            case TYPE_EXPRESSION: {
                auto *expr = dynamic_cast<expression *>(ident);
                auto pre_result0= get_depend_state(expr->get_operand_a_ptr());
                auto pre_result1 = get_depend_state(expr->get_operand_b_ptr());
                pre_result0.insert(pre_result0.end(), pre_result1.begin(), pre_result1.end());
                return pre_result0;
            }
            default:
                assert(true);
        }
        return {};
    }

}