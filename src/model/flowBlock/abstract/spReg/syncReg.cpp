///
/// Created by tanawin on 5/12/2566.
///

#include "syncReg.h"
#include "model/controller/controller.h"

namespace kathryn {

    void SyncReg::com_init() {
        _ctrl->on_sp_reg_init(this, SP_SYNC_REG);
    }

    SyncReg::SyncReg(int size): CtrlFlowRegBase(size,
                                                  false,
                                                  TYPE_STATE_REG,
                                                  false),
                                _up_state      (makeOprVal("upState"      , 1  , 1)),
                                _up_full_state  (makeOprVal("upFullState"  , size,genBiConValRep(true , size))),
                                _down_full_state(makeOprVal("downFullState", size,genBiConValRep(false, size))),
                                _test_wire     (makeOprWire("testSyncWire", size)),
                                _end_expr(&(((*this) | _test_wire) == _up_full_state)),
                                _end_expr_inv(&(~(*_end_expr))),
                                _next_fill_activate_id(0)
    {
        com_init();
        assert(size > 0);
    };

    UpdateEventBase* SyncReg::add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm){
        /// assert(activateCond == nullptr);
        assert(dependState != nullptr);
        Operable* actual_condition = _end_expr_inv;
        if (activateCond != nullptr){
            actual_condition = &((*actual_condition) & (*activateCond));
        }
        /// if endExpr rise, it is neccessary to tel register to rise

        UpdateEventBase* event = create_ue(actual_condition,
                                           dependState,
                                           &_up_state,
                                           Slice({_next_fill_activate_id, _next_fill_activate_id + 1}),
                                           DEFAULT_UE_PRI_INTERNAL_MAX,
                                           cm);
        addUpdateMeta(event);

        /// assign observe wire
        UpdateEventBase* test_event = create_ue(activateCond,
                                                dependState,
                                                &_up_state,
                                                Slice({_next_fill_activate_id, _next_fill_activate_id + 1}),
                                                DEFAULT_UE_PRI_INTERNAL_MAX,
                                                CM_CLK_FREE);
        _test_wire.addUpdateMeta(test_event);
        _next_fill_activate_id++;
        assert(_next_fill_activate_id <= getSlice().getSize());

        return event;
    }

    void SyncReg::make_un_set_state_event(CLOCK_MODE cm) {
        /// unset also testExpr
        auto* event = create_ue(
            nullptr,
            &(((*this) | _test_wire) == _up_full_state),
            &_down_full_state,
            Slice({0, getSlice().getSize()}),
            DEFAULT_UE_PRI_INTERNAL_MIN,
            cm
        );
        addUpdateMeta(event);
    }

    void SyncReg::make_user_rst_event(Operable* userRst, CLOCK_MODE cm) {
        assert(userRst != nullptr);
        auto* event = create_ue(
          nullptr,
          userRst,
          &_down_full_state,
          Slice({0, getSlice().getSize()}),
          DEFAULT_UE_PRI_INTERNAL_MIN, /// we priority set event first rst must be lower
          cm);
        addUpdateMeta(event);
    }

    Operable* SyncReg::generate_end_expr(){
        assert(_next_fill_activate_id == getOperableSlice().getSize());
        return _end_expr;
    }

    std::string gen_conse_binary_value(bool bitVal, int size){
        std::string ret_string = "b";
        std::string fill_val = bitVal ? "1" : "0";
        for (int i = 0; i < size; i++){
            ret_string += fill_val;
        }
        return ret_string;
    }

}