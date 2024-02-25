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

    template<typename... T>
    std::vector<Operable*> getNestVec(Operable& opr, T&... args){
        std::vector<Operable*> deepGet  = getNestVec(args...);
        deepGet.push_back(&opr);
        return deepGet;
    }

    class nest;

    template<typename... T>
    nest& makeNest(Operable& opr, T&... args){
        auto nestList = getNestVec(opr, args...);
        int nestSize = 0;
        for (Operable* slaveOpr: nestList){
            assert(slaveOpr != nullptr);
            nestSize += slaveOpr->getOperableSlice().getSize();

        }
        return _make<nest>("nest", nestSize, nestList);
    }

    class nest : public LogicComp<nest>{
        private:
            /** the higher bit is most significant bit*/
            std::vector<Operable*> _nestList;

        protected:
            void com_init() override;


        public:

            explicit nest(int size, std::vector<Operable*> nestList);

            void com_final() override {};
            /** override assignable*/
            nest& operator <<= (Operable  & b) override {mfAssert(false, "nest don't support this <<= assigment"); assert(false);}
            nest& operator <<= (ull b)         override {mfAssert(false, "nest don't support this <<= assigment"); assert(false);}
            nest& operator =   (Operable  & b) override {mfAssert(false, "nest don't support this   = assigment"); assert(false);}
            nest& operator =   (nest& b)                {mfAssert(false, "nest don't support this   = assigment"); assert(false);}
            nest& operator = (ull b)         override   {mfAssert(false, "nest don't support this = assigment with ull overload"); assert(false);}
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

            /** check short circuit*/
            Operable* checkShortCircuit() override;

            void simStartCurCycle() override;



    };









}

#endif //KATHRYN_NEST_H
