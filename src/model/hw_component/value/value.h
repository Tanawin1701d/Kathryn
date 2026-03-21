//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_VALUE_H
#define KATHRYN_VALUE_H

#include "string"
#include "vector"

#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/abstract/operable.h"
#include "model/hw_component/abstract/slicable.h"
#include "model/hw_component/abstract/logic_comp.h"
#include "util/numberic/num_convert.h"
#include "gen/proxy_hw_comp/value/value_gen.h"

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

        explicit Val(int size, ull raw_value = 0);

        /**
         * override assignable
         * */
        void do_block_asm(Operable& b, Slice des_slice) override {
            mf_assert(false, "val don't support this <<= assigment"); assert(false);
        }
        void do_non_block_asm(Operable& b, Slice des_slice) override {
            mf_assert(false, "val don't support this   = assigment"); assert(false);
        }
        void do_block_asm(Operable& src_opr,
                        std::vector<AssignMeta*>& result_meta_collector,
                        Slice  abs_src_slice,
                        Slice  abs_des_slice) override{
            mf_assert(false, "val don't support this do_block_asm"); assert(false);
        }
        void do_non_block_asm(Operable& src_opr,
                           std::vector<AssignMeta*>& result_meta_collector,
                           Slice  abs_src_slice,
                           Slice  abs_des_slice) override{
            mf_assert(false, "val don't support this do_non_block_asm"); assert(false);
        }

        CLOCK_MODE get_cur_assign_clk_mode() {return CM_CLK_FREE;}

        Val& operator = (Operable& b){ operator_eq(b);                                return *this;}
        Val& operator = (ull b)      { operator_eq(b);                                   return *this;}
        Val& operator = (Val& b)     { if(this == &b){return *this;} operator_eq(b);  return *this;}

        /** assign todo we will assign it later*/
        SliceAgent<Val>& operator() (int start, int stop) override;
        SliceAgent<Val>& operator() (int idx) override;
        SliceAgent<Val>& operator() (Slice sl) override;
        Operable* do_slice(Slice sl) override;
        bool      is_const_opr() override{return true;}
        ull       get_const_opr() override{return _rawValue;}
        Operable* check_short_circuit() override;

        /**override logicc gen base*/
        void create_logic_gen() override;


    };




}

#endif //KATHRYN_VALUE_H
