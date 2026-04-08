//
// Created by tanawin on 3/1/2567.
//

#ifndef MODEL_FLOWBLOCK_ABSTRACT_SPREG_CNTREG_H
#define MODEL_FLOWBLOCK_ABSTRACT_SPREG_CNTREG_H

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
        int       _cnt_bit_sz = -1;
        int       _last_cycle = -1;
        Operable* _idle_val   = nullptr;

    protected:

        void com_init() override;

    public:
        /** constructor*/
        explicit CounterReg(int maxCycle);

        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) override;
        /** add Inc to count*/ /// hold signal is not used because it is only the counter
        void make_inc_event(Operable* upCountEvent, CLOCK_MODE cm);
        /** reset event*/
        void make_un_set_state_event(CLOCK_MODE cm) override;
        /** make user reset event*/
        void  makeUserRstEvent(Operable* rst, CLOCK_MODE cm) override;
        /** generate out expression*/
        Operable* generateEndExpr() override;
        /** oevrride operator to prevent false input*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
        int getLoopCnt() const { return _last_cycle; }
    };

    static int calBitUsedInCounter(int maxNumber){
        assert(maxNumber > 0);
        return (int) log2(maxNumber * 2 - 1);
    }



}

#endif //MODEL_FLOWBLOCK_ABSTRACT_SPREG_CNTREG_H
