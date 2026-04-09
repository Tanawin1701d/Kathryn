///
/// Created by tanawin on 5/12/2566.
///

#ifndef KATHRYN_SYNCREG_H
#define KATHRYN_SYNCREG_H


#include "ctrlFlowRegBase.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/hwComponent/wire/wire.h"


namespace kathryn{
    class Node;

    class SyncReg : public CtrlFlowRegBase{
        Val& _up_state;
        Val& _up_full_state;
        Val& _down_full_state;
        Wire& _test_wire;
        Operable* _end_expr = nullptr;
        Operable* _end_expr_inv = nullptr;
        int _next_fill_activate_id;

    protected:
        /// override data to init state regiter
        void com_init() override;

    public:
        explicit SyncReg(int size = 1);
        /// state register system must handle updateEvent themself
        Reg& operator <<=(Operable& b) override{
            std::cout << "we not support = operator in register";
            return *this;
        }

        /// add prior state that trigger this state
        UpdateEventBase* add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) override;
        /// reset event
        void make_un_set_state_event(CLOCK_MODE cm) override;
        /// make user reset event
        void make_user_rst_event(Operable* userRst, CLOCK_MODE cm) override;
        /// exit expression
        Operable* generate_end_expr() override;
    };

    std::string gen_conse_binary_value(bool bitVal, int size);
}

#endif /// KATHRYN_STATEREG_H
