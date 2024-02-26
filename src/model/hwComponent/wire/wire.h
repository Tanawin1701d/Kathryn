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
        void com_final() override {};

        /**override assignable*/
        [[maybe_unused]]
        Wire& operator <<= (Operable& b) override {mfAssert(false, "wire don't support this <<= assigment");assert(false);};
        Wire& operator <<= (ull b) override {mfAssert(false, "wire don't support this <<= assigment");assert(false);}
        void generateAssMetaForBlocking(Operable& srcOpr,
                                        std::vector<AssignMeta*>& resultMetaCollector,
                                        Slice  absSrcSlice,
                                        Slice  absDesSlice) override{
            mfAssert(false, "wire don't support generateAssMetaForBlocking");
            assert(false);
        };


        Wire& operator =   (Operable& b) override;
        Wire& operator =   (Wire& b);
        Wire& operator =   (ull b) override;
        void generateAssMetaForNonBlocking(Operable& srcOpr,
                                           std::vector<AssignMeta*>& resultMetaCollector,
                                           Slice  absSrcSlice,
                                           Slice  absDesSlice) override;

        /**override operable*/
        Operable& getExactOperable() const override {return *(Operable*)(this);}
        Slice getOperableSlice() const override {return getSlice();}
        /**override slicable*/
        SliceAgent<Wire>& operator() (int start, int stop) override;
        SliceAgent<Wire>& operator() (int idx) override;
        Operable* doSlice(Slice sl) override;
        void makeDefEvent();
        /**override assign call back*/
        Wire& callBackBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) override;
        Wire& callBackNonBlockAssignFromAgent(Operable& b, Slice absSliceOfHost) override;
        void  callBackBlockAssignFromAgent(Operable& srcOpr,
                                           std::vector<AssignMeta*>& resultMetaCollector,
                                           Slice  absSrcSlice,
                                           Slice  absDesSlice) override;
        void  callBackNonBlockAssignFromAgent(Operable& srcOpr,
                                              std::vector<AssignMeta*>& resultMetaCollector,
                                              Slice  absSrcSlice,
                                              Slice  absDesSlice) override;

        Operable* checkShortCircuit() override;


    };

    class WireLogicSim: public LogicSimEngine{
        Wire* _master;
    public:
        WireLogicSim(Wire* master,int sz, VCD_SIG_TYPE sigType, bool simForNext);
        /** override simulation engine */
        void simStartCurCycle() override;
    };

}

#endif //KATHRYN_WIRE_H
