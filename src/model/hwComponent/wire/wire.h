//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_WIRE_H
#define KATHRYN_WIRE_H

#include<iostream>
#include "model/hwComponent/abstract/logicComp.h"
#include "model/controller/conInterf/controllerItf.h"

namespace kathryn{

    class Wire : public LogicComp<Wire>{
    protected:
        void com_init() override;
    public:
        explicit Wire(int size);

        /**override assignable*/
        [[maybe_unused]]
        Wire& operator <<= (Operable& b) override {std::cout << "we not support <<= operator in wire"; return *this;};
        Wire& operator =   (Operable& b) override;
        /**override operable*/
        Operable& getExactOperable() const override {return *(Operable*)(this);}
        Slice getOperableSlice() const override {return getSlice();}
        /**override slicable*/
        SliceAgent<Wire>& operator() (int start, int stop) override;
        SliceAgent<Wire>& operator() (int idx) override;
        /**overide assign call back*/
        Wire& callBackBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) override;
        Wire& callBackNonBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) override;

        //std::vector<std::string> getDebugAssignmentValue() override;


    };

}

#endif //KATHRYN_WIRE_H
