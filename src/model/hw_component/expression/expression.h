//
// Created by tanawin on 28/11/2566.
//

#ifndef KATHRYN_EXPRESSION_H
#define KATHRYN_EXPRESSION_H


#include "memory"
#include "string"
#include "iostream"
#include "utility"

#include "model/controller/con_interf/controller_itf.h"
#include "model/hw_component/abstract/logic_comp.h"
#include "gen/proxy_hw_comp/expression/expr_gen.h"


/**
 * expr_metas is the class that represent the value from hardware
 * component such as register and wire
 *
 * */

namespace kathryn {




    class expression : public LogicComp<expression>{
    friend class expression_sim_engine;
    friend class ExprGen;
    protected:
        bool _valueAssinged = false;
        /** metas data that contain bi operation*/
        const LOGIC_OP _op;
        Operable* _a;
        Operable* _b;

    protected:
        void com_init() override;



    public:
        /** constructor auto get id of the system*/
        explicit expression(LOGIC_OP op,
                            const Operable* a,
                            const Operable* b,
                            int exp_size
                            );
        explicit expression(int exp_size);

        void com_final() override {};
        /** override assignable*/
        void do_block_asm(Operable& src_opr, Slice des_slice) override{
            mf_assert(false, "expr don't support do_block_asm");
            assert(false);
        };
        void do_non_block_asm(Operable& src_opr, Slice des_slice) override;
        void do_non_block_asm_mul_ass_check(Operable& src_opr, Slice des_slice);

        void do_block_asm(Operable& src_opr,
                        std::vector<AssignMeta*>& result_meta_collector,
                        Slice  abs_src_slice,
                        Slice  abs_des_slice) override{
            mf_assert(false, "expr don't support do_block_asm"); assert(false);
        }
        void do_non_block_asm(Operable& src_opr,
                           std::vector<AssignMeta*>& result_meta_collector,
                           Slice  abs_src_slice,
                           Slice  abs_des_slice) override{
            mf_assert(abs_des_slice == get_slice()                    ,
                     "des expression assign wrapper doesn't cover entire expression");
            mf_assert(abs_src_slice.get_size() >= get_slice().get_size(),
                     "src expression assign wrapper doesn't cover entire expression");
            do_global_asm(src_opr, result_meta_collector, abs_src_slice, abs_des_slice, ASM_DIRECT);
        }

        CLOCK_MODE get_cur_assign_clk_mode() override {return CM_CLK_FREE;}

        expression& operator = (Operable& b)  { operator_eq(b);                                return *this;}
        expression& operator = (ull b)        { operator_eq(b);                                   return *this;}
        expression& operator = (expression& b){ if (this == &b){return *this;} operator_eq(b); return *this;}
        /**override operable*/




        /** override slicable*/
        SliceAgent<expression>& operator() (int start, int stop) override;
        SliceAgent<expression>& operator() (int idx) override;
        SliceAgent<expression>& operator() (Slice sl) override;
        Operable* do_slice(Slice sl) override;

        /** override debugg message*/
        //std::vector<std::string> get_debug_assignment_value() override;

        /** get set method */
        LOGIC_OP get_op() const {return _op;};
        Operable*get_operand_a_ptr() const {return _a;}
        Operable*get_operand_b_ptr() const {return _b;}

        /**check short*/
        Operable* check_short_circuit() override;

        void create_logic_gen() override;

    };




}

#endif //KATHRYN_EXPRESSION_H
