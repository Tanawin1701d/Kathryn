//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_WIRE_H
#define KATHRYN_WIRE_H

#include<iostream>
#include "model/hwComponent/abstract/logicComp.h"
#include "model/controller/conInterf/controllerItf.h"
#include "gen/proxyHwComp/wire/wireGen.h"
#include "model/hwComponent/abstract/WireMarker.h"

namespace kathryn{

    class Wire : public LogicComp<Wire>, public WireMarker{
    protected:
        bool _requireDefVal = false;
        void com_init() override;

    public:
        explicit Wire(int size,
            bool requireDefVal = true,
            bool initCom       = true);

        // ~Wire(){
        //     std::cout << "wire is freee" << std::endl;
        // }

        void com_final() override{};

        /**override assignable*/
        void doBlockAsm(Operable& srcOpr, Slice desSlice) override;
        void doNonBlockAsm(Operable& srcOpr, Slice desSlice) override;

        void doBlockAsm(Operable& srcOpr,
                        std::vector<AssignMeta*>& resultMetaCollector,
                        Slice  absSrcSlice,
                        Slice  absDesSlice) override{
            mfAssert(false, "wire don't support this doBlockAsm"); assert(false);
        }
        void doNonBlockAsm(Operable& srcOpr,
                           std::vector<AssignMeta*>& resultMetaCollector,
                           Slice  absSrcSlice,
                           Slice  absDesSlice) override{
            doGlobalAsm(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice, ASM_DIRECT);
        }

        Wire& operator = (Operable& b){ operatorEq(b);                                 return *this;}
        Wire& operator = (ull b)      { operatorEq(b);                                    return *this;}
        Wire& operator = (Wire& b)    { if (this == &b){return *this;} operatorEq(b);  return *this;}

        /**override slicable*/
        SliceAgent<Wire>& operator() (int start, int stop) override;
        SliceAgent<Wire>& operator() (int idx) override;
        SliceAgent<Wire>& operator() (Slice sl) override;
        Operable* doSlice(Slice sl) override;
        void makeDefEvent();
        Operable* checkShortCircuit() override;

        /**override logicc gen base*/
        void createLogicGen() override;

        /** override global input*/
        bool checkIntegrity()                  override;
        void connectToThisIo(WireAuto* wireIo) override;
        Operable* getOprFromGlobIo()           override;

    };



}

#endif //KATHRYN_WIRE_H
