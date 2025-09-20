//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_VALUE_H
#define KATHRYN_VALUE_H

#include<string>
#include<vector>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/logicComp.h"
#include "util/numberic/numConvert.h"
#include "gen/proxyHwComp/value/valueGen.h"

namespace kathryn{

    /** This class act as constant value */
    class Val: public LogicComp<Val>{
        friend class ValSimEngine;
        friend class ValueGen;
    protected:
        int    _size;
        ull   _rawValue;
        /***the actual value will be assigned to val rep*/
        void com_init() override;

    public:
        /** todo we will make value save the value and range more precisly*/
        void com_final() override {};

        explicit Val(int size, ull rawValue = 0);

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

        CLOCK_MODE getCurAssignClkMode() {return CM_CLK_FREE;}

        Val& operator = (Operable& b){ operatorEq(b);                                return *this;}
        Val& operator = (ull b)      { operatorEq(b);                                   return *this;}
        Val& operator = (Val& b)     { if(this == &b){return *this;} operatorEq(b);  return *this;}

        /** assign todo we will assign it later*/
        SliceAgent<Val>& operator() (int start, int stop) override;
        SliceAgent<Val>& operator() (int idx) override;
        SliceAgent<Val>& operator() (Slice sl) override;
        Operable* doSlice(Slice sl) override;
        bool      isConstOpr() override{return true;}
        ull       getConstOpr() override{return _rawValue;}
        Operable* checkShortCircuit() override;

        /**override logicc gen base*/
        void createLogicGen() override;


    };




}

#endif //KATHRYN_VALUE_H
