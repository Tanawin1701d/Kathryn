//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_STATEREG_H
#define KATHRYN_STATEREG_H


#include "model/hwComponent/register/register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"



namespace kathryn {

    class Node;

    class StateReg: public Reg{
        Val upState = Val(1, "b1");
        Val* upFullState;
    protected:
        /**override data to init state regiter*/
        void com_init() override;

    public:

        explicit StateReg(int size = 1): Reg(size, false){
            upFullState = new Val(size, "b" + (std::to_string((1 << size) - 1)));
        };

        /** state register system must handle updateEvent themself*/
        Reg& operator <<= (Operable& b) override {assert(true);}

        UpdateEvent* addUpdateEvent(Operable* dependStateCon = nullptr, int bit = 0){
            auto* event = new UpdateEvent({nullptr,
                                           dependStateCon,
                                           &upState,
                                           Slice({bit, bit + 1}),
                                           9});
            addUpdateMeta(event);
            return event;
        }


        Node* generateStateNode();
        Operable* generateEndExpr();
    };

}

#endif //KATHRYN_STATEREG_H
