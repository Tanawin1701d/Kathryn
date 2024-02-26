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



namespace kathryn{


    std::vector<Operable*> getNestVec();

    struct NestMeta{
        Operable*   opr;
        Assignable* asb;
    };

    template<typename OA, typename... T>
    std::vector<NestMeta> getNestVec(OA& oa, T&... args){
        std::vector<NestMeta> deepGet  = getNestVec(args...);
        deepGet.push_back({&oa, &oa});
        return deepGet;
    }

    class nest;

    template<typename OA, typename... T>
    nest& makeNest(OA& oa, T&... args){
        auto nestList = getNestVec(oa, args...);
        int nestSize = 0;
        for (NestMeta nestMeta: nestList){
            assert(nestMeta.opr != nullptr);
            nestSize += nestMeta.opr->getOperableSlice().getSize();
        }
        return _make<nest>("nest", nestSize, nestList);
    }

    class nest : public LogicComp<nest>{
        friend class NestLogicSim;
        private:
            /** the higher bit is most significant bit*/
            std::vector<NestMeta> _nestList;


        protected:
            void com_init() override;


        public:

            explicit nest(int size, std::vector<NestMeta> nestList);

            void com_final() override {};
            /** override assignable*/
            nest& operator <<= (Operable  & b) override {mfAssert(false, "nest don't support this <<= assigment"); assert(false);}
            nest& operator <<= (ull b)         override {mfAssert(false, "nest don't support this <<= assigment"); assert(false);}
            void generateAssMetaForBlocking(Operable& srcOpr,
                                    std::vector<AssignMeta*>& resultMetaCollector,
                                    Slice  absSrcSlice,
                                    Slice  absDesSlice) override;
            nest& operator =   (Operable  & b) override {mfAssert(false, "nest don't support this   = assigment"); assert(false);}
            nest& operator =   (nest& b)                {mfAssert(false, "nest don't support this   = assigment"); assert(false);}
            nest& operator = (ull b)         override   {mfAssert(false, "nest don't support this = assigment with ull overload"); assert(false);}
            void generateAssMetaForNonBlocking(Operable& srcOpr,
                                               std::vector<AssignMeta*>& resultMetaCollector,
                                               Slice  absSrcSlice,
                                               Slice  absDesSlice) override;

            void generateAssMetaForAll(Operable& srcOpr,
                                       std::vector<AssignMeta*>& resultMetaCollector,
                                       Slice  absSrcSlice,
                                       Slice  absDesSlice,
                                       bool isblockingAsm
                                       );
            /**override operable*/
            [[nodiscard]]
            Slice getOperableSlice() const override  { return getSlice(); }
            [[nodiscard]]
            Operable& getExactOperable() const override { return *(Operable*)(this);};
            [[nodiscard]]



            /** override slicable*/
            SliceAgent<nest>& operator() (int start, int stop) override;
            SliceAgent<nest>& operator() (int idx) override;
            /** call back assignable from client agent*/
            [[maybe_unused]]
            nest& callBackBlockAssignFromAgent(Operable& b, Slice absSlice) override;
            nest& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) override;
            void  callBackBlockAssignFromAgent(Operable& srcOpr,
                                               std::vector<AssignMeta*>& resultMetaCollector,
                                               Slice  absSrcSlice,
                                               Slice  absDesSlice)override;
            void  callBackNonBlockAssignFromAgent(Operable& srcOpr,
                                                  std::vector<AssignMeta*>& resultMetaCollector,
                                                  Slice  absSrcSlice,
                                                  Slice  absDesSlice)override;

            /** check short circuit*/
            Operable* checkShortCircuit() override;

    };


    class NestLogicSim: public LogicSimEngine{
        nest* _master = nullptr;
    public:
        NestLogicSim(nest* master,int sz, VCD_SIG_TYPE sigType, bool simForNext);
        /** override simulation engine */
        void simStartCurCycle() override;
    };










}

#endif //KATHRYN_NEST_H
