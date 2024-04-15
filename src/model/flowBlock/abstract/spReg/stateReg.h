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
        Val& upFullState;
        Val& downFullState;
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
        UpdateEvent* addDependState(Operable* dependState, Operable* activateCond) override;
        /** reset event*/
        void makeUnSetStateEvent() override;
        /** user reset event*/
        void makeUserRstEvent(Operable* rstWire) override { assert(false); /** it is single node; there is no neccessary for reset user event */};
        /** exit expression*/
        Operable* generateEndExpr() override;
        /***resetEvent Status*/
        bool requireResetEvent() override{
            return _rstReq;
        }
    };
}

#endif //KATHRYN_STATEREG_H
