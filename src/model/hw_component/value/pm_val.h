//
// Created by tanawin on 14/1/2025.
//

#ifndef src_model_hwComponent_value_PMVALUE_H
#define src_model_hwComponent_value_PMVALUE_H

#include "string"
#include "vector"

#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/abstract/operable.h"
#include "model/hw_component/abstract/slicable.h"
#include "model/hw_component/abstract/logic_comp.h"
#include "util/numberic/num_convert.h"
#include "gen/proxy_hw_comp/value/value_gen.h"

namespace kathryn{


    class PmVal: public LogicComp<PmVal>{
        friend class PmValSimEngine;
        friend class ParamValGen;

        /** in current version value is supported for 64bit-maximum*/
    protected:
        int _size     = 64;
        ull _rawValue = 0;

        void com_init() override;

    public:

        void com_final() override {};

        explicit PmVal(ull raw_value);

        void set_parameter(ull raw_value){ _rawValue = raw_value;}

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

        PmVal& operator = (Operable& b){ operator_eq(b);                                return *this;}
        PmVal& operator = (ull b)      { operator_eq(b);                                   return *this;}
        PmVal& operator = (PmVal& b) { if(this == &b){return *this;} operator_eq(b);  return *this;}

        SliceAgent<PmVal>& operator() (int start, int stop) override;
        SliceAgent<PmVal>& operator() (int idx) override;
        SliceAgent<PmVal>& operator() (Slice sl) override;
        Operable* do_slice(Slice sl) override;
        bool      is_const_opr() override{return true;}
        ull       get_const_opr() override{return _rawValue;}
        Operable* check_short_circuit() override;

        /**override logic gen base*/
        void create_logic_gen() override;


    };




}

#endif //src_model_hwComponent_value_PMVALUE_H
