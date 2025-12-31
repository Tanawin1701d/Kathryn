//
// Created by tanawin on 25/2/2567.
//

#ifndef KATHRYN_NEST_H
#define KATHRYN_NEST_H



#include <memory>
#include <string>
#include <iostream>
#include <utility>
#include "model/controller/conInterf/controllerItf.h"
#include "model/hwComponent/abstract/logicComp.h"
#include "gen/proxyHwComp/expression/nestGen.h"




namespace kathryn{


    struct NestMeta{
        Operable*   opr1;
        Assignable* asb;
    };

    /////////// for nest vector read and write are acceptable

    std::vector<NestMeta> getNestVec();

    template<typename OA, typename... T>
    std::vector<NestMeta> getNestVec(OA& oa, T&... args){
        std::vector<NestMeta> deepGet  = getNestVec(args...);
        deepGet.push_back({&oa, &oa});
        return deepGet;
    }

    class nest;

    template<typename OA, typename... T>
    nest& makeNest(bool isUserDec,OA& oa, T&... args){
        auto nestList = getNestVec(oa, args...);
        int nestSize = 0;
        for (NestMeta nestMeta: nestList){
            assert(nestMeta.opr1 != nullptr);
            nestSize += nestMeta.opr1->getOperableSlice().getSize();
        }
        return _make<nest>("uncatagorizedYet", "nest", isUserDec, nestSize, nestList);
    }

    /////////// for nest vector read is only acceptable

    std::vector<Operable*> getNestVecReadOnly();

    template<typename... T>
    std::vector<Operable*> getNestVecReadOnly(Operable& firstOperable, T&... args){
        std::vector<Operable*> deepGet = getNestVecReadOnly(args...);
        deepGet.push_back(&firstOperable);
        return deepGet;
    }

    template<typename S, typename... T>
    nest& makeNestReadOnly(bool isUserDec, S& firstOperable, T&... args){
        auto nestList = getNestVecReadOnly(firstOperable, args...);
        return makeNestManReadOnly(isUserDec, nestList);
    }

    //////////// for manual built
    nest& makeNestMan(bool isUser, const std::vector<NestMeta>& groupedMeta);
    nest& makeNestManReadOnly(bool isUser, const std::vector<Operable*>& nestListReadOnly);

    class nest : public LogicComp<nest>{
        friend class NestSimEngine;
        friend class NestGen;
        private:
            /** the higher bit is most significant bit*/
            bool readOnly = false;
            std::vector<NestMeta> _nestList;

        protected:
            void com_init() override;

        public:

            explicit nest(int size, std::vector<NestMeta> nestList);
            explicit nest(int size, const std::vector<Operable*>& nestList);

            void com_final() override {};
            /** override assignable*/
            void doBlockAsm    (Operable& srcOpr, Slice desSlice) override;
            void doNonBlockAsm (Operable& srcOpr, Slice desSlice) override;

            void doBlockAsm    (Operable& srcOpr,
                                std::vector<AssignMeta*>& resultMetaCollector,
                                Slice  absSrcSlice,
                                Slice  absDesSlice) override;
            void doNonBlockAsm (Operable& srcOpr,
                                std::vector<AssignMeta*>& resultMetaCollector,
                                Slice  absSrcSlice,
                                Slice  absDesSlice) override;

            void doGlobalAsm   (Operable& srcOpr,
                                std::vector<AssignMeta*>& resultMetaCollector,
                                Slice  absSrcSlice,
                                Slice  absDesSlice,
                                ASM_TYPE asmType) override {assert(false);/**disable this function*/}

            CLOCK_MODE getCurAssignClkMode() {assert(false); return CM_CLK_UNUSED;}

            nest& operator = (Operable& b){ operatorEq(b);                                return *this;}
            nest& operator = (ull b)      { operatorEq(b);                                   return *this;}
            nest& operator = (nest& b)    { if (this == &b){return *this;} operatorEq(b); return *this;}

        /**get which netlist element match that bit Idx */
            [[maybe_unused]]
            int getNetListIdxThatMatch(int bitIdx);

            void doNestGlobalAsm(Operable& srcOpr,
                                 std::vector<AssignMeta*>& resultMetaCollector,
                                 Slice  absSrcSlice,
                                 Slice  absDesSlice,
                                 bool isblockingAsm
                                       );

            /** override slicable*/
            SliceAgent<nest>& operator() (int start, int stop) override;
            SliceAgent<nest>& operator() (int idx) override;
            SliceAgent<nest>& operator() (Slice sl) override;
            Operable* doSlice(Slice sl) override;

            std::vector<NestMeta>& getNestList() {return _nestList;}

            /** check short circuit*/
            Operable* checkShortCircuit() override;

            /**override logicc gen base*/
            void createLogicGen() override;

    };

}

#endif //KATHRYN_NEST_H
