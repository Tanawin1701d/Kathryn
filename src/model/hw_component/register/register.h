//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_REGISTER_H
#define KATHRYN_REGISTER_H

#include "model/hw_component/abstract/logic_comp.h"
#include "model/controller/con_interf/controller_itf.h"
#include "gen/proxy_hw_comp/register/reg_gen.h"
#include "model/hw_component/abstract/wire_marker.h"

namespace kathryn{



    class Reg : public LogicComp<Reg>, public WireMarker{

    protected:
        void com_init() override;


    public:
        explicit Reg(int size, bool init_com = true, HW_COMPONENT_TYPE hw_type = TYPE_REG, bool required_alloc_check = true);
        ~Reg() override = default;

        void com_final() override {};

        /** assignable override*/
        void do_block_asm(Operable& src_opr, Slice des_slice) override;
        void do_non_block_asm(Operable& src_opr, Slice des_slice) override;
        void do_global_asm(Operable& src_opr, Slice des_slice, ASM_TYPE asm_type) override;
            /////// for block is declar in assign base class
        void do_block_asm(Operable& src_opr,
                           std::vector<AssignMeta*>& result_meta_collector,
                           Slice  abs_src_slice,
                           Slice  abs_des_slice) override{
            Assignable::do_global_asm(src_opr, result_meta_collector, abs_src_slice, abs_des_slice, ASM_DIRECT);
        };
        void do_non_block_asm(Operable& src_opr,
                           std::vector<AssignMeta*>& result_meta_collector,
                           Slice  abs_src_slice,
                           Slice  abs_des_slice) override{
            Assignable::do_global_asm(src_opr, result_meta_collector, abs_src_slice, abs_des_slice, ASM_EQ_DEPNODE);
        };

        CLOCK_MODE get_cur_assign_clk_mode() override {return GET_CLOCK_MODE();}

        Reg& operator = (Operable& b){ operator_eq(b);                                return *this;}
        Reg& operator = (ull b)      { operator_eq(b);                                   return *this;}
        Reg& operator = (Reg& b)     { if (this == &b){return *this;} operator_eq(b); return *this;}

        /** Slicable*/
        SliceAgent<Reg>& operator() (int start, int stop) override;
        SliceAgent<Reg>& operator() (int idx) override;
        SliceAgent<Reg>& operator() (Slice sl) override;
        Operable* do_slice(Slice sl) override;
        /**make rst_event*/
        void make_reset_event(ull value = 0, CLOCK_MODE cm = CM_POSEDGE);
        void make_def_event(ull def_value = 0) override;
        /** return type*/
        Operable* check_short_circuit() override;

        /**override logicc gen base*/
        void create_logic_gen() override;

        /** override global input*/
        bool check_integrity()                     override;
        Operable*get_opr_from_glob_io_ptr()              override;
        Assignable*get_asb_from_wire_marker_ptr()        override;

    };




}

#endif //KATHRYN_REGISTER_H
