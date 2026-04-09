//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_STATEREG_H
#define KATHRYN_STATEREG_H


#include "ctrlFlowRegBase.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"
#include "model/flowBlock/abstract/nodes/node.h"


namespace kathryn {

    class Node;

    class StateReg: public CtrlFlowRegBase{
        bool _rstReq = true; //// did reset event require
        makeVal(stateRegUpFull  , 1, 1);
        makeVal(stateRegDownFull, 1, 0);
    protected:
        /**override data to init state regiter*/
        void com_init() override;
    public:
        explicit StateReg(bool rstReq = true);
        /** state register system must handle updateEvent themself*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) override;
        /** reset event*/
        void make_un_set_state_event(CLOCK_MODE cm) override;
        /** user reset event*/
        void make_user_rst_event(Operable* rstWire, CLOCK_MODE cm) override { assert(false); /** it is single node; there is no neccessary for reset user event */};
        /** exit expression*/
        Operable* generate_end_expr() override;
        /***resetEvent Status*/
        bool require_reset_event() override{
            return _rstReq;
        }
    };
}

#endif //KATHRYN_STATEREG_H
