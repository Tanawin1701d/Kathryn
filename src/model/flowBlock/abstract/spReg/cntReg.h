//
// Created by tanawin on 3/1/2567.
//

#ifndef KATHRYN_WAITREG_H
#define KATHRYN_WAITREG_H

#include <iostream>
#include <cmath>
#include "ctrlFlowRegBase.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/value/value.h"
#include "model/hwComponent/abstract/makeComponent.h"

namespace kathryn{

    /**
     *                    2 1 0
     * [ | | | | | | | | | | | ]
     *
     * */

    class CounterReg : public CtrlFlowRegBase{
    private:
        /**wait cycle from use declare*/
        int       _cntBitSz  = -1;
        int       _lastCycle = -1;
        Operable* _idleVal   = nullptr;

    protected:

        void com_init() override;

    public:
        /** constructor*/
        explicit CounterReg(int maxCycle);

        /** add prior state that trigger this state*/
        UpdateEventBase* addDependState(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) override;
        /** add Inc to count*/ /// hold signal is not used because it is only the counter
        void makeIncEvent(Operable* upCountEvent, CLOCK_MODE cm);
        /** reset event*/
        void makeUnSetStateEvent(CLOCK_MODE cm) override;
        /** make user reset event*/
        void  makeUserRstEvent(Operable* rst, CLOCK_MODE cm) override;
        /** generate out expression*/
        Operable* generateEndExpr() override;
        /** oevrride operator to prevent false input*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
        int getLoopCnt() const { return _lastCycle; }
    };

    static int calBitUsedInCounter(int maxNumber){
        assert(maxNumber > 0);
        return (int) log2(maxNumber * 2 - 1);
    }



}

#endif //KATHRYN_WAITREG_H
