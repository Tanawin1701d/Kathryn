//
// Created by tanawin on 18/1/2567.
//


#include "assignable.h"
#include "make_component.h"
#include "model/hw_component/value/value.h"
#include "model/flow_block/abstract/nodes/asm_node.h"

namespace kathryn{

    /**assign mode control*/


    Operable& get_match_assign_operable(ull value, const int size){
        make_val(ass_user_auto_val, size, value);
        return ass_user_auto_val;
    }

    void Assignable::do_global_asm(
            Operable& src_opr,
            std::vector<AssignMeta*>& result_meta_collector,
            Slice  abs_src_slice,
            Slice  abs_des_slice,
            ASM_TYPE asm_type
            ){
        /** check slice integrity*/
        assert(src_opr.get_operable_slice().is_contain(abs_src_slice));
        assert(get_assign_slice().is_contain(abs_des_slice));
        ///assert(abs_src_slice == abs_des_slice);

        Slice desire_src_slice   = abs_src_slice.get_match_size_sub_slice(abs_des_slice);
        Slice desire_des_slice   = abs_des_slice.get_match_size_sub_slice(abs_src_slice);

        Operable* exact_src_opr  = &src_opr.get_exact_operable();
        Operable* sliced_src_opr = exact_src_opr->do_slice(desire_src_slice);

        result_meta_collector.push_back(
                generate_assign_meta( *sliced_src_opr, desire_des_slice, asm_type, get_cur_assign_clk_mode())
        );
    }


    AsmNode* Assignable::generate_basic_node(Operable& src_opr, Slice des_slice, ASM_TYPE asm_type){

        assert(des_slice.get_size() <= src_opr.get_operable_slice().get_size());

        auto* ass_meta = generate_assign_meta(src_opr,des_slice,asm_type, get_cur_assign_clk_mode());
        auto* asm_node = new AsmNode(ass_meta);
        return asm_node;

    }

    //
    // bool Assignable::check_des_is_fully_assign_and_equal(){
    //
    //     if (_updateMeta.empty()){ return true; }
    //     Slice des_is = get_assign_slice();
    //     for (UpdateEvent* ud_ptr: _updateMeta){
    //         if (ud_ptr->des_update_slice == des_is){
    //             continue;
    //         }
    //         return false;
    //     }
    //     return true;
    // }




}