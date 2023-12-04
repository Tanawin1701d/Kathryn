//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_WIRE_H
#define KATHRYN_WIRE_H

#include <iostream>
#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"

namespace kathryn{

    class Wire : public Assignable<Wire>, public Operable,
            public Slicable<Wire>,
            public AssignCallbackFromAgent<Wire>,
            public Identifiable,
            public HwCompControllerItf{
    protected:
        void com_init() override;
    public:
        explicit Wire(int size);

        /**override assignable*/
        [[maybe_unused]]
        Wire& operator <<= (Operable& b) override {std::cout << "we not support <<= operator in wire";};
        Wire& operator =   (Operable& b) override;
        /**override operable*/
        Operable& getExactOperable() override {return *(Operable*)(this);}
        Slice getOperableSlice() override {return getSlice();}
        /**override slicable*/
        SliceAgent<Wire>& operator() (int start, int stop) override;
        SliceAgent<Wire>& operator() (int idx) override;
        /**overide assign call back*/
        Wire& callBackBlockAssignFromAgent(Operable& b, Slice absSlice) override;
        Wire& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) override;


    };

}

#endif //KATHRYN_WIRE_H
