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
        Operable*   opr;
        Assignable* asb;
    };

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
            assert(nestMeta.opr != nullptr);
            nestSize += nestMeta.opr->getOperableSlice().getSize();
        }
        return _make<nest>("nest", isUserDec, nestSize, nestList);
    }

    class nest : public LogicComp<nest>{
        friend class NestSimEngine;
        friend class NestGen;
        private:
            /** the higher bit is most significant bit*/
            std::vector<NestMeta> _nestList;

        protected:
            void com_init() override;

        public:

            explicit nest(int size, std::vector<NestMeta> nestList);

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


    class NestSimEngine: public LogicSimEngine{
        nest* _master = nullptr;
    public:
        NestSimEngine(nest* master, VCD_SIG_TYPE sigType);
        void        proxyBuildInit() override;
        std::string createOp()       override;

    };










}

#endif //KATHRYN_NEST_H
