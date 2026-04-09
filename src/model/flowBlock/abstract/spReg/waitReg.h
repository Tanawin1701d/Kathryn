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
      *
      * conditional wait state register
      *
      * */

    class CondWaitStateReg : public CtrlFlowRegBase{

        makeVal(_up_state  ,1, 1);
        makeVal(_down_state,1, 0);
        Operable* _cond_opr = nullptr;

    protected:

        void com_init() override;

    public:
        /** constructor*/
        explicit CondWaitStateReg(Operable* condOpr);
        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) override;
        /** generate reset event*/
        void make_un_set_state_event(CLOCK_MODE cm) override;
        /** make user reset event*/
        void  make_user_rst_event(Operable* rst, CLOCK_MODE cm) override;
        /** generate out expression*/
        Operable* generate_end_expr() override;
        /** oevrride operator to prevent false input*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
    };

    /**
      *
      * cycle count wait state register
      *
      * */


    /**
     *                    2 1 0
     * [ | | | | | | | | | | | ]
     *                        ^---------- state bit
     * */

    class CycleWaitStateReg : public CtrlFlowRegBase{
    private:
        /**wait cycle meta data*/

        int               _wait_cycle    = -1;
        int               _cnt_bit_sz    = -1;
        static const int  STATE_SIZE     = 1;
        const int         TOTAL_BIT_SIZE = -1;
        const int         START_VAL      = 0b11;
        /**when counter is reached exit expression will be set*/
        Operable* _idle_cnt     = nullptr;
        Operable* _start_cnt    = nullptr;
        Operable* _end_cnt      = nullptr;
    protected:

        void com_init() override;

    public:
        /** constructor*/
        explicit CycleWaitStateReg(int waitCycle);
        explicit CycleWaitStateReg(Operable* endCnt);

        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* dependState, Operable* activateCond, CLOCK_MODE cm) override;
        /** add Inc to count*/
        void make_inc_state_event(Operable* holdSignal, CLOCK_MODE cm);
        /** reset event*/
        void make_un_set_state_event(CLOCK_MODE cm) override;
        /** make user reset event*/
        void  make_user_rst_event(Operable* rst, CLOCK_MODE cm) override;
        /** generate out expression*/
        Operable* generate_end_expr() override;
        /** oevrride operator to prevent false input*/
        Reg& operator <<= (Operable& b) override {
            std::cout << "we not support = operator in register";
            return *this;
        }
    };

    static int cal_bit_used(int maxNumber){
        assert(maxNumber > 0);
        int amtNumberUsed = maxNumber + 1;
        /** time 2 and minus 1 to make ceiling of the number*/
        return (int) log2(amtNumberUsed * 2 - 1);
    }



}

#endif //KATHRYN_WAITREG_H
