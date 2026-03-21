//
// Created by tanawin on 18/9/25.
//

#include "reg_slot.h"
#include "model/controller/controller.h"

namespace kathryn{

    //////// RegSlotDynSliceAgent

    RegSlotDynSliceAgent& RegSlotDynSliceAgent::operator <<=(Operable& rhs_opr){
        _masterSlot.do_block_asm   (rhs_opr, _required_idx, ASM_DIRECT, _isOH);
        return *this;
    }

    RegSlotDynSliceAgent& RegSlotDynSliceAgent::operator <<=(ull rhs_val){
        _masterSlot.do_block_asm   (rhs_val, _required_idx, ASM_DIRECT, _isOH);
        return *this;
    }

    RegSlotDynSliceAgent& RegSlotDynSliceAgent::operator =(Operable& rhs_opr){
        _masterSlot.do_non_block_asm(rhs_opr, _required_idx, ASM_EQ_DEPNODE, _isOH);
        return *this;
    }

    RegSlotDynSliceAgent& RegSlotDynSliceAgent::operator =(ull rhs_val){
        _masterSlot.do_non_block_asm(rhs_val, _required_idx, ASM_EQ_DEPNODE, _isOH);
        return *this;
    }


    /////// RegSlot


    RegSlot::RegSlot(const SlotMeta& slot_meta,
                const std::vector<Reg*>& regs
        ):
        Slot(slot_meta){
            /** this is used to initialize from RegSlot slice*/
            mf_assert(slot_meta.get_num_field_ptr() == regs.size(), "field_metas size not match with regs size");
            _regs = regs;
            for(Reg* reg: _regs){
                _hwFieldMetas.push_back({reg, reg});
            }
        }

    RegSlot::RegSlot(const SlotMeta& slot_meta, const std::string& prefix_name):
    Slot(slot_meta){
        /** this is used to initialize from slice*/
        RegSlot::init_hw_structure(prefix_name);
    }

    RegSlot::RegSlot(const std::vector<std::string>& field_names,
            const std::vector<int>&                  field_sizes,
            const std::string&                       prefix_name):
    Slot(field_names, field_sizes){
        /** create new reg*/
        RegSlot::init_hw_structure(prefix_name);
    }

    void RegSlot::init_hw_structure(const std::string& prefix_name){
        for(int idx = 0; idx < _meta.get_num_field_ptr(); idx++){
            FieldMeta field_meta = _meta.get_copy_field(idx);
            mf_assert(field_meta._size > 0, "field " + field_meta._name + " is not pass integrity test");
            Reg* new_reg = &m_opr_reg(prefix_name +
                "colIdx_" + std::to_string(idx) +
                "_" + field_meta._name,
                field_meta._size);
            _regs.push_back(new_reg);
            _hwFieldMetas.push_back({new_reg, new_reg});
        }
    }


    /**
     * The main function to overwrite the assignment
     */
    void RegSlot::do_glob_asm(
        const Slot& rhs,
        const std::vector<int>& src_match_idxs,
        const std::vector<int>& des_match_idxs,
        const std::vector<int>& except_idxs,
        ASM_TYPE asm_type) {

        AsmNode* asm_node = gen_grp_asm_node(
            rhs,
            src_match_idxs,
            des_match_idxs,
            except_idxs,
            asm_type
        );
        do_glob_asm(asm_node);
    }

    void RegSlot::do_glob_asm(Operable& src_opr,
                            Operable& required_idx,
                            ASM_TYPE asm_type,
                            bool is_oh) {
        AsmNode* asm_node = gen_grp_asm_node(src_opr, required_idx, asm_type, is_oh);
        do_glob_asm(asm_node);
    }

    void RegSlot::do_glob_asm(AsmNode* asm_node) {
        assert(asm_node != nullptr);
        ModelController* ctrl = get_controller_ptr();
        assert(ctrl != nullptr);
        ctrl->on_reg_update(
            asm_node,
            nullptr
        );
    }

    void RegSlot::make_reset_event(ull reset_value, CLOCK_MODE cm){
        for (Reg* reg: _regs){
            reg->make_reset_event(reset_value, cm);
        }
    }

    void RegSlot::make_reset_event(int col_idx, ull reset_value, CLOCK_MODE cm){
        mf_assert(is_valid_idx(col_idx), "index out of range to get " + std::to_string(col_idx));
        _regs[col_idx]->make_reset_event(reset_value, cm);
    }

    void RegSlot::make_reset_event(const std::string& col_name, ull reset_value, CLOCK_MODE cm){
        int col_idx = get_idx_ptr(col_name);
        make_reset_event(col_idx, reset_value, cm);
    }


    /***
     *  static slicing
     */

    /** single slicing*/
    Reg& RegSlot::operator () (int idx){
        mf_assert(is_valid_idx(idx), "index out of range to get " + std::to_string(idx));
        return *_regs[idx];
    }

    Reg& RegSlot::operator () (const std::string& field_name){
        int idx = get_idx_ptr(field_name);
        mf_assert(_meta.is_valid_idx(idx), "field name " + field_name + " not found");
        return *_regs[idx];
    }

    RegSlot RegSlot::operator() (int start, int end){
        SlotMeta new_slot_meta = _meta(start, end);
        std::vector<Reg*> new_regs;
        for (int idx = start; idx < end; idx++){
            new_regs.push_back(_regs[idx]);
        }
        return RegSlot(new_slot_meta, new_regs);
    }

    /** range slicing*/
    RegSlot RegSlot::operator() (const std::string& start_field,
                        const std::string& end_field){
        int idx    = get_idx_ptr(start_field);
        int end_idx = get_idx_ptr(end_field) + 1;

        mf_assert(is_valid_idx(idx)   , "field name " + start_field + " not found");
        mf_assert(is_valid_idx(end_idx), "field name " + end_field + " not found");

        return (*this)(idx, end_idx);
    }

    /** individual slicing*/
    RegSlot RegSlot::operator() (const std::vector<int>& field_idxs){
        SlotMeta new_slot_meta = _meta(field_idxs);
        std::vector<Reg*> new_regs;
        for (auto idx: field_idxs){
            new_regs.push_back(_regs[idx]);
        }
        return RegSlot(new_slot_meta, new_regs);
    }

    RegSlot RegSlot::operator() (const std::vector<std::string>& field_names){
        std::vector<int> field_idxs = get_idx_ptrs(field_names);
        return (*this)(field_idxs);
    }

    RegSlot RegSlot::operator + (const RegSlot& rhs){
        SlotMeta new_slot_meta = get_meta() + rhs.get_meta();
        std::vector<Reg*> new_regs = _regs;
        _regs.insert(_regs.end(), rhs._regs.begin(), rhs._regs.end());
        return {new_slot_meta, new_regs};
    }


    /**
     *  dynamic indexing
     */
    RegSlotDynSliceAgent RegSlot::operator[](Operable& required_idx){
        return {*this, required_idx, false};
    }

    RegSlotDynSliceAgent RegSlot::operator[](const OH& required_idx){
        return {*this, required_idx._idx, true};
    }

    /** it will match by name*/
    RegSlot& RegSlot::operator <<= (const Slot& rhs){
        do_block_asm(rhs, ASM_DIRECT);
        return *this;
    }

    RegSlot& RegSlot::operator <<= (const RegSlot& rhs){
        do_block_asm(rhs, ASM_DIRECT);
        return *this;
    }

    RegSlot& RegSlot::operator = (const Slot& rhs){
        do_non_block_asm(rhs, ASM_EQ_DEPNODE);
        return *this;
    }

    RegSlot& RegSlot::operator = (const RegSlot& rhs){
        do_non_block_asm(rhs, ASM_EQ_DEPNODE);
        return *this;
    }





}