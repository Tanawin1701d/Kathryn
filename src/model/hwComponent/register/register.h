//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_REGISTER_H
#define KATHRYN_REGISTER_H

#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"

namespace kathryn{



    class Reg : public Assignable<Reg>, public Operable,
            public Slicable<Reg>,
            public AssignCallbackFromAgent<Reg>,
            public Identifiable,
            public HwCompControllerItf{

    protected:
        void com_init() override;

    public:
        explicit Reg(int size);
        ~Reg();

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

        Reg& callBackBlockAssignFromAgent(Operable& b, Slice absSlice) override;
        Reg& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) override;



    };


}

#endif //KATHRYN_REGISTER_H
