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

    /** This class act as constant value */
    class Val: public LogicComp<Val>{
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
            LogicComp({0, size}, TYPE_VAL,VST_INTEGER, false, false),
            _size(size),
            rawValue(NumConverter::cvtStrToValRep(size, args...))
            {
                assert(size > 0);
                com_init();
                initSim();
            }

        explicit Val(int size):
            LogicComp({0, size}, TYPE_VAL,
                      VST_INTEGER,
                      false,
                      false),
            _size(size),
            rawValue(NumConverter::cvtStrToValRep(size, 0))
            {
                assert(size > 0);
                com_init();
                initSim();
            }

        explicit Val(const ValRep& val):
            LogicComp({0, val.getLen()}, TYPE_VAL,VST_INTEGER, false, false),
            _size(val.getLen()),
            rawValue(val)
            {
                assert(val.getLen() > 0);
                com_init();
                initSim();
            }

        /**
         * override assignable
         * */
        Val& operator <<= (Operable& b) override { mfAssert(false, "val don't support this <<= assigment");assert(false);}
        Val& operator =   (Operable& b) override { mfAssert(false, "val don't support this   = assigment");assert(false);}
        Val& operator =   (Val&      b)          { mfAssert(false, "val don't support this   = assigment");assert(false);}


        /** override operable*/
        [[nodiscard]]
        Slice     getOperableSlice() const override { return Slice{0, _size};}
        [[nodiscard]]
        Operable& getExactOperable() const override {return (Operable &) *this;}

        /** assign todo we will assign it later*/
        SliceAgent<Val>& operator() (int start, int stop) override;
        SliceAgent<Val>& operator() (int idx) override;

        Val& callBackBlockAssignFromAgent(Operable& b, Slice absSlice) override {assert(false);};
        Val& callBackNonBlockAssignFromAgent(Operable& b, Slice absSlice) override{assert(false);};

        /** override simulator*/
        void simStartCurCycle() override;
        void simExitCurCycle() override;
        /** init sim*/
        void initSim();

        Operable* checkShortCircuit() override;


    };

}

#endif //KATHRYN_VALUE_H
