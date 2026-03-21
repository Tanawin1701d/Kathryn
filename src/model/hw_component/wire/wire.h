//
// Created by tanawin on 29/11/2566.
//

#ifndef KATHRYN_WIRE_H
#define KATHRYN_WIRE_H

#include "iostream"
#include "model/hw_component/abstract/logic_comp.h"
#include "model/controller/con_interf/controller_itf.h"
#include "gen/proxy_hw_comp/wire/wire_gen.h"
#include "model/hw_component/abstract/wire_marker.h"

namespace kathryn{

    class Wire : public LogicComp<Wire>, public WireMarker{
    protected:
        bool _requireDefVal = false;
        void com_init() override;

    public:
        explicit Wire(int size,
            bool require_def_val = true,
            bool init_com       = true);

        void com_final() override{};

        /**override assignable*/
        void do_block_asm(Operable& src_opr, Slice des_slice) override;
        void do_non_block_asm(Operable& src_opr, Slice des_slice) override;

        void do_block_asm(Operable& src_opr,
                        std::vector<AssignMeta*>& result_meta_collector,
                        Slice  abs_src_slice,
                        Slice  abs_des_slice) override{
            mf_assert(false, "wire don't support this do_block_asm"); assert(false);
        }
        void do_non_block_asm(Operable& src_opr,
                           std::vector<AssignMeta*>& result_meta_collector,
                           Slice  abs_src_slice,
                           Slice  abs_des_slice) override{
            do_global_asm(src_opr, result_meta_collector, abs_src_slice, abs_des_slice, ASM_DIRECT);
        }

        CLOCK_MODE get_cur_assign_clk_mode() override { return CM_CLK_FREE;};

        Wire& operator = (Operable& b){ operator_eq(b);                                 return *this;}
        Wire& operator = (ull b)      { operator_eq(b);                                    return *this;}
        Wire& operator = (Wire& b)    { if (this == &b){return *this;} operator_eq(b);  return *this;}

        /**override slicable*/
        SliceAgent<Wire>& operator() (int start, int stop) override;
        SliceAgent<Wire>& operator() (int idx) override;
        SliceAgent<Wire>& operator() (Slice sl) override;
        Operable* do_slice(Slice sl) override;
        void make_def_event(ull def_val = 0) override;
        Operable* check_short_circuit() override;

        /**override logicc gen base*/
        void create_logic_gen() override;
        /** override global input*/
        bool check_integrity()                  override;
        Operable*get_opr_from_glob_io_ptr()           override;
        Assignable*get_asb_from_wire_marker_ptr()     override;

    };



}

#endif //KATHRYN_WIRE_H
