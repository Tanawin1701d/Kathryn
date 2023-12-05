//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_STATEREG_H
#define KATHRYN_STATEREG_H


#include "model/hwComponent/register/register.h"
#include "model/hwComponent/value/value.h"

namespace kathryn {

    class StateReg: public Reg{
        Val upState = Val(1, "b1");
    protected:
        /**override data to init state regiter*/
        void com_init() override;

    public:

        explicit StateReg(int size = 1): Reg(size){};

        /** state register system must handle updateEvent themself*/
        Reg& operator <<= (Operable& b) override {assert(true);}

        UpdateEvent* genUpdateEvent(Operable* dependStateCon = nullptr){
            auto* event = new UpdateEvent({nullptr,
                                           dependStateCon,
                                                  &upState,
                                                  9});
            addUpdateMeta(event);
            return event;
        }

    };


}

#endif //KATHRYN_STATEREG_H
