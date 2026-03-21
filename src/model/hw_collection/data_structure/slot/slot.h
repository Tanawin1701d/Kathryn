//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H

#include "utility"

#include "slot_meta.h"
#include "model/flow_block/abstract/nodes/asm_node.h"
#include "model/hw_component/abstract/logic_comp.h"
#include "model/hw_component/register/register.h"
#include "model/hw_collection/data_structure/indexing/index.h"

namespace kathryn{

    struct HwFieldMeta{
        Operable* _opr   = nullptr;
        Assignable* _asb = nullptr;
    };

    class Slot{

    protected:
        SlotMeta _meta;
        std::vector<HwFieldMeta> _hwFieldMetas;

    public:

        Slot()= default;

        Slot(const std::vector<std::string>& field_names,
             const std::vector<int>&         field_sizes
        ):
        _meta(field_names, field_sizes){}

        explicit Slot(SlotMeta  meta):
        _meta(std::move(meta)){}

        virtual ~Slot() = default;

        virtual void init_hw_structure(const std::string& prefix_name){ assert(false);}

        ////////// get static data


        [[nodiscard]] bool is_there_field(const std::string& field_name) const{
            return get_idx_ptr(field_name) != -1;
        }

        FieldMeta& field_ref_at(int idx){
            return _meta(idx);
        }

        [[nodiscard]]
        FieldMeta field_at(int idx) const{
            return _meta.get_copy_field(idx);
        }

        HwFieldMeta& hw_field_ref_at(int idx){
            mf_assert(is_valid_idx(idx), "get hw Field at " + std::to_string(idx) + " out of range");
            return _hwFieldMetas[idx];
        }

        HwFieldMeta& hw_field_ref_at(const std::string& field_name){
            int field_id = get_idx_ptr(field_name);
            mf_assert(is_valid_idx(field_id), "field name " + field_name + " not found");
            return hw_field_ref_at(field_id);
        }

        HwFieldMeta hw_field_at(int idx) const{
            mf_assert(is_valid_idx(idx), "get hw Field at " + std::to_string(idx) + " out of range");
            return _hwFieldMetas[idx];
        }

        HwFieldMeta hw_field_at(const std::string& field_name) const{
            int field_id = get_idx_ptr(field_name);
            mf_assert(is_valid_idx(field_id), "field name " + field_name + " not found");
            return hw_field_at(field_id);
        }

        void add_hw_field_meta(HwFieldMeta hw_field_meta){
            _hwFieldMetas.push_back(std::move(hw_field_meta));
        }

        SlotMeta get_meta() const{
            return _meta;
        }

        int get_max_bit_width(){
            return _meta.get_max_bit_width();
        }

        int get_idx_ptr(std::string field_name) const{
            return _meta.get_idx_ptr(std::move(field_name));
        }

        std::vector<int> get_idx_ptrs(std::vector<std::string> field_names) const{
            return _meta.get_idx_ptrs(std::move(field_names));
        }

        int get_num_field_ptr() const{
            return _meta.get_num_field_ptr();
        }

        bool is_sufficient_idx(int idx_size){
            return _meta.is_sufficient_idx(idx_size);
        }

        bool is_valid_idx(int idx) const{
            return _meta.is_valid_idx(idx);
        }
        bool check_valid_range(int start, int stop){
            return _meta.is_valid_range(start, stop);
        }

        std::pair<std::vector<int>, std::vector<int>> match_by_name(const Slot& rhs){
            return _meta.match_by_name(rhs._meta);
        }


        //////// assign system
        AssignMeta* gen_assign_meta(Operable& src_opr, Assignable& des_asb,
                                 ASM_TYPE asm_type) const{

            mf_assert(des_asb.get_assign_slice().get_size() <= src_opr.get_operable_slice().get_size(),
            "the size of src_opr is too small to assign to des_asb");
            Slice src_slice = src_opr.get_operable_slice();
            Slice des_slice = des_asb.get_assign_slice();

            UpdateEventBasic* ueb = create_ue_helper(
                &src_opr,
                des_slice.get_match_size_sub_slice(src_slice),
                -1,
                des_asb.get_cur_assign_clk_mode(),
                true
            );

            AssignMeta* ass_meta = new AssignMeta(des_asb.get_update_meta(),
                                                 ueb,
                                                 asm_type);
            return ass_meta;
        }

        AssignMeta* gen_assign_meta(Operable& src_opr, int field_idx,
                                  ASM_TYPE asm_type) const{
            auto [des_opr, des_asb] = hw_field_at(field_idx);
            return gen_assign_meta(src_opr, *des_asb, asm_type);
        }

        std::vector<AssignMeta*> gen_assign_meta_for_all(const Slot& src_slot, ASM_TYPE asm_type) const{
            assert(get_num_field_ptr() == src_slot.get_num_field_ptr());
            std::vector<AssignMeta*> result_collector;
            for (int des_idx = 0; des_idx < src_slot.get_num_field_ptr(); des_idx++){
                auto [des_opr, des_asb] = hw_field_at(des_idx);
                auto [src_opr, src_asb] = src_slot.hw_field_at(des_idx);

                AssignMeta* ass_meta = gen_assign_meta(*src_opr, *des_asb, asm_type);
                result_collector.push_back(ass_meta);
            }

            return result_collector;
        }

        std::vector<AssignMeta*> gen_assign_meta_for_all(std::vector<Operable*> src_operables, ASM_TYPE asm_type) const{
            assert(get_num_field_ptr() == src_operables.size());
            std::vector<AssignMeta*> result_collector;
            for (int des_idx = 0; des_idx < get_num_field_ptr(); des_idx++){
                auto [des_opr, des_asb] = hw_field_at(des_idx);
                Operable* src_opr = src_operables[des_idx];
                assert(src_opr != nullptr);
                AssignMeta* ass_meta = gen_assign_meta(*src_opr, *des_asb, asm_type);
                result_collector.push_back(ass_meta);
            }

            return result_collector;
        }

        std::vector<AssignMeta*> gen_assign_meta_for_all(
            const Slot& src_slot,
            const std::vector<int>& src_match_idxs,
            const std::vector<int>& des_match_idxs,
            const std::vector<int>& except_idxs,
            ASM_TYPE asm_type){

            std::vector<AssignMeta*> result_collector;

            mf_assert(src_match_idxs.size() == des_match_idxs.size(),
                "size of src_match_idxs and des_match_idxs must be equal");

            for (int idx = 0; idx < src_match_idxs.size(); idx++){

                int src_idx    = src_match_idxs[idx];
                int des_idx    = des_match_idxs[idx];


                if (std::find(except_idxs.begin(),except_idxs.end(),src_idx) != except_idxs.end()){
                    ///// encounter except list
                    continue;
                }

                auto [des_opr, des_asb] = hw_field_ref_at(des_idx);
                auto [src_opr, src_asb] = src_slot.hw_field_at(src_idx);

                AssignMeta* ass_meta = gen_assign_meta(*src_opr, *des_asb, asm_type);
                result_collector.push_back(ass_meta);
            }

            return result_collector;
        }

        std::vector<AssignMeta*> gen_assign_meta_for_all(Operable& src_opr, ASM_TYPE asm_type){
            std::vector<AssignMeta*> result_collector;

            for (int des_idx = 0; des_idx < get_num_field_ptr(); des_idx++){
                auto [des_opr, des_asb] = hw_field_ref_at(des_idx);

                AssignMeta* ass_meta = gen_assign_meta(src_opr, *des_asb, asm_type);
                result_collector.push_back(ass_meta);
            }
            return result_collector;
        }


        AsmNode* gen_grp_asm_node (
            const Slot& src_slot,
            const std::vector<int>& src_match_idxs,
            const std::vector<int>& des_match_idxs,
            const std::vector<int>& except_idxs,
            ASM_TYPE asm_type){
            std::vector<AssignMeta*> result_collector
            = gen_assign_meta_for_all(src_slot, src_match_idxs, des_match_idxs, except_idxs, asm_type);
            auto* asm_node = new AsmNode(result_collector);
            return asm_node;

        }

        /** gen the group the asm node for all matched*/
        AsmNode* gen_grp_asm_node(const Slot& src_slot, ASM_TYPE asm_type){

            std::vector<AssignMeta*> result_collector = gen_assign_meta_for_all(src_slot, asm_type);
            auto* asm_node = new AsmNode(result_collector);
            return asm_node;

        }

        AsmNode* gen_grp_asm_node(
            Operable& src_opr,
            Operable& required_idx,
            ASM_TYPE  asm_type,
            bool      is_oh
        ){

            std::vector<AssignMeta*> result_collector = gen_assign_meta_for_all(src_opr, asm_type);

            auto* asm_node = new AsmNode(result_collector);

            for (int des_idx = 0; des_idx < get_num_field_ptr(); des_idx++){
                Operable* idx_check_cond = nullptr;
                if (is_oh){
                    idx_check_cond = &(required_idx.sl(des_idx));
                }else{
                    idx_check_cond = &(required_idx == des_idx);
                }

                asm_node->add_specific_pre_condition(idx_check_cond, des_idx);
            }
            return asm_node;
        }

        static AsmNode* gen_grp_asm_node(
        const std::vector<AssignMeta*> & assign_metas,
        const std::vector<Operable*>   & pre_conditions
        ){
            assert(assign_metas.size() == pre_conditions.size());
            auto* asm_node = new AsmNode(assign_metas);
            for (int idx = 0; idx < pre_conditions.size(); idx++){
                asm_node->add_specific_pre_condition(pre_conditions[idx], idx);
            }
            return asm_node;
        }

        static AsmNode* gen_grp_asm_node(
        const std::vector<AssignMeta*> & assign_metas){
            auto* asm_node = new AsmNode(assign_metas);
            return asm_node;
        }





        virtual void do_glob_asm(
            const Slot& rhs,
            const std::vector<int>& src_match_idxs,
            const std::vector<int>& des_match_idxs,
            const std::vector<int>& except_idxs,
            ASM_TYPE asm_type){assert(false);}

        virtual void do_glob_asm(
            Operable& src_opr,
            Operable& required_idx,
            ASM_TYPE  asm_type,
            bool      is_oh
        ){assert(false);}

        virtual void do_glob_asm(
            AsmNode* asm_node
        ){assert(false);}


        /** block assignment */

        //////// static assign
        virtual void do_block_asm(const Slot& rhs, const std::vector<int>& except_idxs, ASM_TYPE asm_type){
            auto [src_match_idxs, des_match_idxs] = match_by_name(rhs);
            do_glob_asm(rhs, src_match_idxs, des_match_idxs, {}, asm_type);
        }
        virtual void do_block_asm(const Slot& rhs, const std::vector<std::string>& except_names, ASM_TYPE asm_type){
            std::vector<int> excpet_idxs = get_idx_ptrs(except_names);
            do_block_asm(rhs, excpet_idxs, asm_type);
        }
        virtual void do_block_asm(const Slot& rhs, ASM_TYPE asm_type){
            do_block_asm(rhs, std::vector<int>{}, asm_type);
        }
        //////// dynamic assign
        virtual void do_block_asm(Operable& src_opr, Operable& required_idx, ASM_TYPE asm_type, bool is_oh){
            do_glob_asm(src_opr, required_idx, asm_type, is_oh);
        }

        virtual void do_block_asm(ull src_val, Operable& required_idx, ASM_TYPE asm_type, bool is_oh){
            Operable& my_src_opr = get_match_assign_operable(src_val, get_max_bit_width());
            do_block_asm(my_src_opr, required_idx, asm_type, is_oh);
        }
        /** non block assignment */

        //////// static assign
        virtual void do_non_block_asm(const Slot& rhs, const std::vector<int>& except_idxs, ASM_TYPE asm_type){
            auto [src_match_idxs, des_match_idxs] = match_by_name(rhs);
            do_glob_asm(rhs, src_match_idxs, des_match_idxs, {}, asm_type);
        }
        virtual void do_non_block_asm(const Slot& rhs, const std::vector<std::string>& except_names, ASM_TYPE asm_type){
            std::vector<int> excpet_idxs = get_idx_ptrs(except_names);
            do_non_block_asm(rhs, excpet_idxs, asm_type);
        }
        virtual void do_non_block_asm(const Slot& rhs, ASM_TYPE asm_type){
            do_non_block_asm(rhs, std::vector<int>{}, asm_type);
        }

        //////// dynamic assign

        virtual void do_non_block_asm(Operable& src_opr, Operable& required_idx, ASM_TYPE asm_type, bool is_oh){
            do_glob_asm(src_opr, required_idx, asm_type, is_oh);
        }

        virtual void do_non_block_asm(ull src_val, Operable& required_idx, ASM_TYPE asm_type, bool is_oh){
            Operable& my_src_opr = get_match_assign_operable(src_val, get_max_bit_width());
            do_non_block_asm(my_src_opr, required_idx, asm_type, is_oh);
        }

    };

    class SlotDynSliceAgent{
    protected:
        Slot&     _masterSlot;
        Operable& _required_idx;
        bool      _isOH =false;


    public:

        explicit SlotDynSliceAgent(Slot& master, Operable& required_idx, bool is_oh):
        _masterSlot(master),
        _required_idx(required_idx),
        _isOH(is_oh)
        {}

        Operable& v(){

            int target_width = _masterSlot.get_max_bit_width();

            Wire* result_wire = &make_opr_wire("slot_slice", target_width);

            bool is_used_as_def = true;
            //// the target structure to update
            UpdatePool& update_pool = result_wire->get_update_meta();

            for (int idx = 0; idx < _masterSlot.get_num_field_ptr(); idx++){
                FieldMeta field_meta = _masterSlot.field_ref_at(idx);
                ///// we do only the target port
                if (field_meta._size != target_width ){
                    continue;
                }
                ////// we require the first val as the default value
                Operable* activate_cond = nullptr;
                int       assign_pri    = DEFAULT_UE_PRI_MIN;
                if (!is_used_as_def){
                    if(_isOH){
                        activate_cond = &_required_idx.sl(idx);
                    }else{
                        activate_cond = &(_required_idx == idx);
                    }
                    assign_pri    = DEFAULT_UE_PRI_USER;
                }
                ////// create update event
                UpdateEventBase* result_up_event = create_ue_helper(
                activate_cond,
                nullptr,
                _masterSlot.hw_field_ref_at(idx)._opr,
                result_wire->get_operable_slice(),
                assign_pri,
                CM_CLK_FREE,
                false
                );

                // UpdateEventBasic* ueb = new
                // UpdateEventBasic(_masterSlot.hw_field_ref_at(idx)._opr,
                //     result_wire->get_operable_slice(), assign_pri,CM_CLK_FREE);
                //
                // UpdateEventCond* uec = new UpdateEventCond();
                // uec->add_sub_stmt(activate_cond, ueb);

                update_pool.add_update_event(result_up_event);
                is_used_as_def = false;
            }

            /////// set default value

            return *result_wire;
        }


    };

}


#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOT_H

