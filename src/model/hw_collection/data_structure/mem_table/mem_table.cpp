//
// Created by tanawin on 7/12/25.
//

#include "mem_table.h"


namespace kathryn{

    WireSlot MemTable::gen_dyn_wire_slot(Operable* index){

        assert(index != nullptr);
        assert(get_meta().get_num_field_ptr() == _memStorages.size());
        WireSlot result_wire_slot(get_meta());
        /////// recruite opr and assign meta
        std::vector<Operable*> src_oprs;
        for (int col_idx = 0; col_idx < get_meta().get_num_field_ptr(); col_idx++){
            Operable* col_opr = &((*_memStorages[col_idx])[*index]);
            assert(col_opr != nullptr);
            src_oprs.push_back(col_opr);
        }
        std::vector<AssignMeta*> result_collector = result_wire_slot.gen_assign_meta_for_all(src_oprs, ASM_DIRECT);

        /////// assign to build the system
        AsmNode* asm_node = WireSlot::gen_grp_asm_node(result_collector);
        asm_node->dry_assign();
        delete asm_node;
        return result_wire_slot;

    }

    void MemTable::do_glob_asm(Slot& rhs, Operable& required_idx, ASM_TYPE asm_type){


        SlotMeta src_slot_meta = rhs.get_meta();

        ///// source index match index, destination index match index
        auto [src_match_idxs, des_match_idxs] = get_meta().match_by_name(src_slot_meta);

        assert(src_match_idxs.size() == des_match_idxs.size());

        std::vector<AssignMeta*> result_collector;

        for(int idx = 0; idx < src_match_idxs.size(); idx++){
            auto src_idx = src_match_idxs[idx];
            auto des_idx = des_match_idxs[idx];
            /** get src des hw */
            Operable* src_value = rhs.hw_field_ref_at(src_idx)._opr;
            MemBlock* target_mem_blk = _memStorages[des_idx];
            MemBlockEleHolder* mem_ele_holder = &(*target_mem_blk)[required_idx];
            /** buidl */
            AssignMeta* ass_meta =
            mem_ele_holder ->
             generate_assign_meta(*src_value,
                                {0, target_mem_blk->get_width_size()},
                                asm_type,
                                mem_ele_holder->get_cur_assign_clk_mode()
                               );

            result_collector.push_back(ass_meta);

        }

        auto* asm_node = new AsmNode(result_collector);
        asm_node->dry_assign();
        delete asm_node;


    }


}