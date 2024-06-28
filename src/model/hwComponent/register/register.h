//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_REGISTER_H
#define KATHRYN_REGISTER_H

#include "model/hwComponent/abstract/logicComp.h"
#include "model/controller/conInterf/controllerItf.h"
#include "gen/proxyHwComp/register/regGen.h"
#include "model/hwComponent/abstract/globIo.h"

namespace kathryn{



    class Reg : public LogicComp<Reg>, public GlobIo{

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

        Reg& operator = (Operable& b){ operatorEq(b);                                return *this;}
        Reg& operator = (ull b)      { operatorEq(b);                                   return *this;}
        Reg& operator = (Reg& b)     { if (this == &b){return *this;} operatorEq(b); return *this;}

        /** Slicable*/
        SliceAgent<Reg>& operator() (int start, int stop) override;
        SliceAgent<Reg>& operator() (int idx) override;
        SliceAgent<Reg>& operator() (Slice sl) override;
        Operable* doSlice(Slice sl) override;
        /**make rstEvent*/
        void makeResetEvent();
        /** return type*/
        Operable* checkShortCircuit() override;

        /**override logicc gen base*/
        void createLogicGen() override;

        /** override global input*/
        std::string  getGlobIoName()           override;
        bool checkIntegrity()                override;
        void connectToThisIo(WireIo* wireIo) override;
        Operable* getOprFromGlobIo()         override;
    };

    class RegSimEngine: public LogicSimEngine{
        Reg* _master = nullptr;
    public:
        RegSimEngine(Reg* master,VCD_SIG_TYPE sigType);
    };


}

#endif //KATHRYN_REGISTER_H
