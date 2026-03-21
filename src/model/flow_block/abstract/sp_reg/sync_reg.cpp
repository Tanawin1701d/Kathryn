//
// Created by tanawin on 5/12/2566.
//

#include "sync_reg.h"
#include "model/controller/controller.h"

namespace kathryn {

    void SyncReg::com_init() {
        ctrl->on_sp_reg_init(this, SP_SYNC_REG);
    }

    SyncReg::SyncReg(int size): CtrlFlowRegBase(size,
                                                  false,
                                                  TYPE_STATE_REG,
                                                  false),
                                up_state      (make_opr_val("up_state"      , 1  , 1)),
                                up_full_state  (make_opr_val("up_full_state"  , size,gen_bi_con_val_rep(true , size))),
                                down_full_state(make_opr_val("down_full_state", size,gen_bi_con_val_rep(false, size))),
                                test_wire     (make_opr_wire("test_sync_wire", size)),
                                end_expr(&(((*this) | test_wire) == up_full_state)),
                                end_expr_inv(&(~(*end_expr))),
                                next_fill_activate_id(0)
    {
        com_init();
        assert(size > 0);
    };

    UpdateEventBase* SyncReg::add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm){
        ///assert(activate_cond == nullptr);
        assert(depend_state != nullptr);
        Operable* actual_condition = end_expr_inv;
        if (activate_cond != nullptr){
            actual_condition = &( (*actual_condition) & (*activate_cond));
        }
        /** if end_expr rise, it is neccessary to tel register to rise*/

        UpdateEventBase* event = create_ue(actual_condition,
                                          depend_state,
                                          &up_state,
                                          Slice({next_fill_activate_id, next_fill_activate_id + 1}),
                                          DEFAULT_UE_PRI_INTERNAL_MAX,
                                          cm);
        add_update_meta(event);

        ////// assign observe wire
        UpdateEventBase* test_event = create_ue(activate_cond,
                                              depend_state,
                                              &up_state,
                                              Slice({next_fill_activate_id, next_fill_activate_id + 1}),
                                              DEFAULT_UE_PRI_INTERNAL_MAX,
                                              CM_CLK_FREE);
        test_wire.add_update_meta(test_event);
        next_fill_activate_id++;
        assert(next_fill_activate_id <= get_slice().get_size());

        return event;
    }

    void SyncReg::make_un_set_state_event(CLOCK_MODE cm) {

        ////// unset also test_expr
        auto* event = create_ue(
            nullptr,
            &(((*this) | test_wire) == up_full_state),
            &down_full_state,
            Slice({0, get_slice().get_size()}),
            DEFAULT_UE_PRI_INTERNAL_MIN,
            cm
        );
        add_update_meta(event);
    }

    void SyncReg::make_user_rst_event(Operable* user_rst, CLOCK_MODE cm) {
        assert(user_rst != nullptr);
        auto* event = create_ue(
          nullptr,
          user_rst,
          &down_full_state,
          Slice({0, get_slice().get_size()}),
          DEFAULT_UE_PRI_INTERNAL_MIN,////// we priority set event first rst must be lower
          cm);
        add_update_meta(event);
    }

    Operable* SyncReg::generate_end_expr(){
        assert(next_fill_activate_id == get_operable_slice().get_size());
        return end_expr;
    }

    std::string gen_conse_binary_value(bool bit_val, int size){

        std::string ret_string = "b";
        std::string fill_val = bit_val ? "1" : "0";
        for (int i = 0; i < size; i++ ){
            ret_string += fill_val;
        }
        return ret_string;

    }

}