//
// Created by tanawin on 14/1/2025.
//

#ifndef src_model_hwComponent_value_PMVALUE_H
#define src_model_hwComponent_value_PMVALUE_H

#include<string>
#include<vector>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/logicComp.h"
#include "util/numberic/numConvert.h"
#include "gen/proxyHwComp/value/valueGen.h"

namespace kathryn{


    class PmVal: public LogicComp<PmVal>{
        friend class PmValSimEngine;

        /** in current version value is supported for 64bit-maximum*/
    protected:
        int _size     = 64;
        ull _rawValue = 0;

        void com_init() override;

    public:

        void com_final() override {};

        explicit PmVal(ull rawValue);

        void setParameter(ull rawValue){ _rawValue = rawValue;}

        /**
         * override assignable
         * */
        void doBlockAsm(Operable& b, Slice desSlice) override {
            mfAssert(false, "val don't support this <<= assigment"); assert(false);
        }
        void doNonBlockAsm(Operable& b, Slice desSlice) override {
            mfAssert(false, "val don't support this   = assigment"); assert(false);
        }
        void doBlockAsm(Operable& srcOpr,
                        std::vector<AssignMeta*>& resultMetaCollector,
                        Slice  absSrcSlice,
                        Slice  absDesSlice) override{
            mfAssert(false, "val don't support this doBlockAsm"); assert(false);
        }
        void doNonBlockAsm(Operable& srcOpr,
                           std::vector<AssignMeta*>& resultMetaCollector,
                           Slice  absSrcSlice,
                           Slice  absDesSlice) override{
            mfAssert(false, "val don't support this doNonBlockAsm"); assert(false);
        }

        PmVal& operator = (Operable& b){ operatorEq(b);                                return *this;}
        PmVal& operator = (ull b)      { operatorEq(b);                                   return *this;}
        PmVal& operator = (PmVal& b) { if(this == &b){return *this;} operatorEq(b);  return *this;}

        SliceAgent<PmVal>& operator() (int start, int stop) override;
        SliceAgent<PmVal>& operator() (int idx) override;
        SliceAgent<PmVal>& operator() (Slice sl) override;
        Operable* doSlice(Slice sl) override;
        bool      isConstOpr() override{return true;}
        ull       getConstOpr() override{return _rawValue;}
        Operable* checkShortCircuit() override;

        /**override logic gen base*/
        void createLogicGen() override;


    };




}

#endif //src_model_hwComponent_value_PMVALUE_H
