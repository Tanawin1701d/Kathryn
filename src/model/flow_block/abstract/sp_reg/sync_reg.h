//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_SYNCREG_H
#define KATHRYN_SYNCREG_H


#include "ctrl_flow_reg_base.h"
#include "model/hw_component/expression/expression.h"
#include "model/hw_component/value/value.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/hw_component/wire/wire.h"


namespace kathryn {

    class Node;

    class SyncReg: public CtrlFlowRegBase{
        Val& up_state;
        Val& up_full_state;
        Val& down_full_state;
        Wire& test_wire;
        Operable* end_expr = nullptr;
        Operable* end_expr_inv = nullptr;
        int next_fill_activate_id;
    protected:
        /**override data to init state regiter*/
        void com_init() override;
    public:
        explicit SyncReg(int size = 1);
        /** state register system must handle update_event themself*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm) override;
        /** reset event*/
        void make_un_set_state_event(CLOCK_MODE cm) override;
        /** make user reset event*/
        void make_user_rst_event(Operable* user_rst, CLOCK_MODE cm) override;
        /** exit expression*/
        Operable* generate_end_expr() override;
    };

    std::string gen_conse_binary_value(bool bit_val, int size);

}

#endif //KATHRYN_STATEREG_H
