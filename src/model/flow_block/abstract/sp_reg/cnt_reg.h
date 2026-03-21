//
// Created by tanawin on 3/1/2567.
//

#ifndef MODEL_FLOWBLOCK_ABSTRACT_SPREG_CNTREG_H
#define MODEL_FLOWBLOCK_ABSTRACT_SPREG_CNTREG_H

#include "iostream"
#include "cmath"
#include "ctrl_flow_reg_base.h"
#include "model/hw_component/expression/expression.h"
#include "model/hw_component/value/value.h"
#include "model/hw_component/abstract/make_component.h"

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
        explicit CounterReg(int max_cycle);

        /** add prior state that trigger this state*/
        UpdateEventBase* add_depend_state(Operable* depend_state, Operable* activate_cond, CLOCK_MODE cm) override;
        /** add Inc to count*/ /// hold signal is not used because it is only the counter
        void make_inc_event(Operable* up_count_event, CLOCK_MODE cm);
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
        int get_loop_cnt() const { return _lastCycle; }
    };

    static int cal_bit_used_in_counter(int max_number){
        assert(max_number > 0);
        return (int) log2(max_number * 2 - 1);
    }



}

#endif //MODEL_FLOWBLOCK_ABSTRACT_SPREG_CNTREG_H
