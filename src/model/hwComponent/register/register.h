//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_REGISTER_H
#define KATHRYN_REGISTER_H

#include "model/hwComponent/abstract/logicComp.h"
#include "model/controller/conInterf/controllerItf.h"
#include "gen/proxyHwComp/register/regGen.h"
#include "model/hwComponent/abstract/WireMarker.h"

namespace kathryn{



    class Reg : public LogicComp<Reg>, public WireMarker{

    protected:
        void com_init() override;


    public:
        explicit Reg(int size, bool initCom = true, HW_COMPONENT_TYPE hwType = TYPE_REG, bool requiredAllocCheck = true);
        ~Reg() override = default;

        void com_final() override {};

        /** assignable override*/
        void doBlockAsm(Operable& srcOpr, Slice desSlice) override;
        void doNonBlockAsm(Operable& srcOpr, Slice desSlice) override;
        void doGlobalAsm(Operable& srcOpr, Slice desSlice, ASM_TYPE asmType) override;
            /////// for block is declar in assign base class
        void doBlockAsm(Operable& srcOpr,
                           std::vector<AssignMeta*>& resultMetaCollector,
                           Slice  absSrcSlice,
                           Slice  absDesSlice) override{
            Assignable::doGlobalAsm(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice, ASM_DIRECT);
        };
        void doNonBlockAsm(Operable& srcOpr,
                           std::vector<AssignMeta*>& resultMetaCollector,
                           Slice  absSrcSlice,
                           Slice  absDesSlice) override{
            Assignable::doGlobalAsm(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice, ASM_EQ_DEPNODE);
        };

        CLOCK_MODE getCurAssignClkMode() override {return GET_CLOCK_MODE();}

        Reg& operator = (Operable& b){ operatorEq(b);                                return *this;}
        Reg& operator = (ull b)      { operatorEq(b);                                   return *this;}
        Reg& operator = (Reg& b)     { if (this == &b){return *this;} operatorEq(b); return *this;}

        /** Slicable*/
        SliceAgent<Reg>& operator() (int start, int stop) override;
        SliceAgent<Reg>& operator() (int idx) override;
        SliceAgent<Reg>& operator() (Slice sl) override;
        Operable* doSlice(Slice sl) override;
        /**make rstEvent*/
        void makeResetEvent(ull value = 0, CLOCK_MODE cm = CM_POSEDGE);
        /** return type*/
        Operable* checkShortCircuit() override;

        /**override logicc gen base*/
        void createLogicGen() override;

        /** override global input*/
        bool checkIntegrity()                     override;
        Operable* getOprFromGlobIo()              override;
        Assignable* getAsbFromWireMarker()        override;

    };




}

#endif //KATHRYN_REGISTER_H
