//
// Created by tanawin on 3/1/2567.
//

#ifndef KATHRYN_WAITREG_H
#define KATHRYN_WAITREG_H

#include "iostream"
#include "cmath"
#include "ctrl_flow_reg_base.h"
#include "model/hw_component/expression/expression.h"
#include "model/hw_component/value/value.h"
#include "model/hw_component/abstract/make_component.h"

namespace kathryn{

    /**
      *
      * conditional wait state register
      *
      * */

    class CondWaitStateReg : public CtrlFlowRegBase{

        make_val(_upState  ,1, 1);
        make_val(_downState,1, 0);
        Operable* _condOpr = nullptr;

    protected:

        void com_init() override;

    public:
        /** constructor*/
        explicit CondWaitStateReg(Operable* cond_opr);
        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm) override;
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

        int               _waitCycle = -1;
        int               _cntBitSz     = -1;
        static const int  state_size = 1;
        const int         _totalBitSize = -1;
        const int         start_val = 0b11;
        /**when counter is reached exit expression will be set*/
        Operable* IdleCnt     = nullptr;
        Operable* _startCnt    = nullptr;
        Operable* _endCnt      = nullptr;
    protected:

        void com_init() override;

    public:
        /** constructor*/
        explicit CycleWaitStateReg(int wait_cycle);
        explicit CycleWaitStateReg(Operable* end_cnt);

        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm) override;
        /** add Inc to count*/
        void make_inc_state_event(Operable* hold_signal, CLOCK_MODE cm);
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

    static int cal_bit_used(int max_number){
        assert(max_number > 0);
        int amt_number_used = max_number + 1;
        /** time 2 and minus 1 to make ceiling of the number*/
        return (int) log2(amt_number_used * 2 - 1);
    }



}

#endif //KATHRYN_WAITREG_H
