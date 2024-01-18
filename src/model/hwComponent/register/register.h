//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_REGISTER_H
#define KATHRYN_REGISTER_H

#include "model/hwComponent/abstract/logicComp.h"
#include "model/controller/conInterf/controllerItf.h"

namespace kathryn{



    class Reg : public LogicComp<Reg>{

    protected:
        void com_init() override;

    public:
        explicit Reg(int size, bool initCom = true, HW_COMPONENT_TYPE hwType = TYPE_REG, bool requiredAllocCheck = true);
        ~Reg() override = default;

        /** assignable override*/
        Reg& operator <<= (Operable& b) override;
        [[maybe_unused]]
        Reg& operator =   (Operable& b) override;

        /** Operable override*/
        Slice getOperableSlice() const override {return getSlice();};
        Operable& getExactOperable() const override {return *(Operable*)this; };

        /** Slicable*/
        SliceAgent<Reg>& operator() (int start, int stop) override;
        SliceAgent<Reg>& operator() (int idx) override;

        /** return type*/

        Reg& callBackBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) override;
        Reg& callBackNonBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) override;

        /** override simulation engine */
        void simStartCurCycle() override;

        void simExitCurCycle() override;
    };


}

#endif //KATHRYN_REGISTER_H
