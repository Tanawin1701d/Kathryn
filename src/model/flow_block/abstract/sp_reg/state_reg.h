//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_STATEREG_H
#define KATHRYN_STATEREG_H


#include "ctrl_flow_reg_base.h"
#include "model/hw_component/expression/expression.h"
#include "model/hw_component/value/value.h"
#include "model/flow_block/abstract/nodes/node.h"


namespace kathryn {

    class Node;

    class StateReg: public CtrlFlowRegBase{
        bool _rstReq = true; //// did reset event require
        make_val(state_reg_up_full  , 1, 1);
        make_val(state_reg_down_full, 1, 0);
    protected:
        /**override data to init state regiter*/
        void com_init() override;
    public:
        explicit StateReg(bool rst_req = true);
        /** state register system must handle update_event themself*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm) override;
        /** reset event*/
        void make_un_set_state_event(CLOCK_MODE cm) override;
        /** user reset event*/
        void make_user_rst_event(Operable* rst_wire, CLOCK_MODE cm) override { assert(false); /** it is single node; there is no neccessary for reset user event */};
        /** exit expression*/
        Operable* generate_end_expr() override;
        /***reset_event Status*/
        bool require_reset_event() override{
            return _rstReq;
        }
    };
}

#endif //KATHRYN_STATEREG_H
