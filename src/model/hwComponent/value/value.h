//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_VALUE_H
#define KATHRYN_VALUE_H

#include<string>
#include<vector>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"
#include "model/hwComponent/abstract/slicable.h"
#include "model/hwComponent/abstract/logicComp.h"
#include "util/numberic/numConvert.h"

namespace kathryn{

    class Val;
    class ValLogicSim: public LogicSimEngine{
        Val* _master = nullptr;
    public:
        ValLogicSim(Val* master,int sz, VCD_SIG_TYPE sigType, bool simForNext);
        /** override simulation engine */
        void initSim();
        void simStartCurCycle() override;
        void simExitCurCycle() override;


    };
    /** This class act as constant value */
    class Val: public LogicComp<Val>{
        friend class ValLogicSim;
    protected:
        int    _size;
        ValRep rawValue;
        /***the actual value will be assigned to val rep*/

        void com_init() override;

    public:
        /** todo we will make value save the value and range more precisly*/
        void com_final() override {};

        template<typename... Args>
        explicit Val(int size, Args... args):
            LogicComp({0, size},
                      TYPE_VAL,
                      new ValLogicSim(this,size,VST_INTEGER, false),
                      false),
            _size(size),
            rawValue(NumConverter::createValRep(size, args...))
            {
                assert(size > 0);
                com_init();
                ((ValLogicSim*)_simEngine)->initSim();
                AssignOpr::setMaster(this);
                AssignCallbackFromAgent::setMaster(this);
            }

        explicit Val(int size):
            LogicComp({0, size},
                      TYPE_VAL,
                      new ValLogicSim(this,size,VST_INTEGER, false),
                      false),
            _size(size),
            rawValue(NumConverter::createValRep(size, 0))
            {
                assert(size > 0);
                com_init();
                ((ValLogicSim*)_simEngine)->initSim();
                AssignOpr::setMaster(this);
                AssignCallbackFromAgent::setMaster(this);
            }

        explicit Val(const ValRep& val):
            LogicComp({0, val.getLen()},
                      TYPE_VAL,
                      new ValLogicSim(this,val.getLen(),VST_INTEGER, false),
                      false),
            _size(val.getLen()),
            rawValue(val)
            {
                assert(val.getLen() > 0);
                com_init();
                ((ValLogicSim*)_simEngine)->initSim();
                AssignOpr::setMaster(this);
                AssignCallbackFromAgent::setMaster(this);
            }

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

        Val& operator = (Operable& b){ operatorEq(b);                                return *this;}
        Val& operator = (ull b)      { operatorEq(b);                                   return *this;}
        Val& operator = (Val& b)     { if(this == &b){return *this;} operatorEq(b);  return *this;}

        /** assign todo we will assign it later*/
        SliceAgent<Val>& operator() (int start, int stop) override;
        SliceAgent<Val>& operator() (int idx) override;
        SliceAgent<Val>& operator() (Slice sl) override;
        Operable* doSlice(Slice sl) override;
        Operable* checkShortCircuit() override;


    };


}

#endif //KATHRYN_VALUE_H
