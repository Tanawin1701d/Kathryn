//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_STATEREG_H
#define KATHRYN_STATEREG_H


#include "ctrlFlowRegBase.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"
#include "model/FlowBlock/abstract/nodes/node.h"


namespace kathryn {

    class Node;

    class StateReg: public CtrlFlowRegBase{
        Val upState;
        Val upFullState;
        Val downFullState;
        int nextFillActivateId;
    protected:
        /**override data to init state regiter*/
        void com_init() override;
    public:
        explicit StateReg(int size = 1);
        /** state register system must handle updateEvent themself*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
        /** add prior state that trigger this state*/
        UpdateEvent* addDependState(Operable* dependState, Operable* activateCond) override;
        /** reset event*/
        void makeResetEvent() override;
        /** exit expression*/
        Operable* generateEndExpr() override;
    };

    std::string genConseBinaryValue(bool bitVal, int size);

}

#endif //KATHRYN_STATEREG_H
