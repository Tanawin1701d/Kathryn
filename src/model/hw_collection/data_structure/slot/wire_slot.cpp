//
// Created by tanawin on 18/9/25.
//

#include "wire_slot.h"
#include "model/controller/controller.h"

namespace kathryn{

    WireSlotDynSliceAgent& WireSlotDynSliceAgent::operator <<=(Operable&){
        mf_assert(false, "wire slot not support <<= operator");
        return *this;
    }

    WireSlotDynSliceAgent& WireSlotDynSliceAgent::operator <<=(ull){
        mf_assert(false, "wire slot not support <<= operator");
        return *this;
    }

    WireSlotDynSliceAgent& WireSlotDynSliceAgent::operator =(Operable& rhs_opr){
        _masterSlot.do_non_block_asm(rhs_opr, _required_idx, ASM_DIRECT, _isOH);
        return *this;
    }

    WireSlotDynSliceAgent& WireSlotDynSliceAgent::operator =(ull rhs_val){
        _masterSlot.do_non_block_asm(rhs_val, _required_idx, ASM_DIRECT, _isOH);
        return *this;
    }

    WireSlot::WireSlot(const SlotMeta& slot_meta,
                const std::vector<Wire*>& wires
        ):
        Slot(slot_meta){
            /** this is used to initialize from slice*/
            mf_assert(slot_meta.get_num_field_ptr() == wires.size(), "field_metas size not match with wires size");
            _wires = wires;
            for(Wire* wire: _wires){
                _hwFieldMetas.push_back({wire, wire});
            }
        }

    WireSlot::WireSlot(const SlotMeta& slot_meta, const std::string& prefix_name):
        Slot(slot_meta){
            /** this is used to initialize from slice*/
            WireSlot::init_hw_structure(prefix_name);
        }

    WireSlot::WireSlot(const std::vector<std::string>& field_names,
                const std::vector<int>&         field_sizes,
                const std::string&              prefix_name):
        Slot(field_names, field_sizes){
        WireSlot::init_hw_structure(prefix_name);
    }

    WireSlot::WireSlot(const Slot& rhs, const std::string& prefix_name):
    Slot(rhs.get_meta()){
        WireSlot::init_hw_structure(prefix_name);
        AsmNode* asm_node = gen_grp_asm_node(rhs, ASM_DIRECT);
        asm_node->dry_assign();
        delete asm_node;
    }

    void WireSlot::init_hw_structure(const std::string& prefix_name){

        for(int idx = 0; idx < _meta.get_num_field_ptr(); idx++){
            FieldMeta field_meta = _meta.get_copy_field(idx);
            mf_assert(field_meta._size > 0, "field " + field_meta._name + " is not pass integrity test");
            Wire* new_wire = &m_opr_wire(prefix_name + "_" +field_meta._name, field_meta._size);
            _wires.push_back(new_wire);
            _hwFieldMetas.push_back({new_wire, new_wire});
        }

    }

        /**
             * The main function to overwrite the assignment
             */
    void WireSlot::do_glob_asm(
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

    void WireSlot::do_glob_asm(Operable& src_opr,
                             Operable& required_idx,
                             ASM_TYPE  asm_type,
                             bool      is_oh) {
        AsmNode* asm_node = gen_grp_asm_node(src_opr, required_idx, asm_type, is_oh);
        do_glob_asm(asm_node);
    }


    /***
     *  static slicing
     */

    /** single slicing*/
    Wire& WireSlot::operator () (int idx){
        mf_assert(is_valid_idx(idx), "index out of range to get " + std::to_string(idx));
        return *_wires[idx];
    }

    Wire& WireSlot::operator () (const std::string& field_name){
        int idx = get_idx_ptr(field_name);
        mf_assert(_meta.is_valid_idx(idx), "field name " + field_name + " not found");
        return *_wires[idx];
    }

    WireSlot WireSlot::operator() (int start, int end){
        SlotMeta new_slot_meta = _meta(start, end);
        std::vector<Wire*> new_wires;
        for (int idx = start; idx < end; idx++){
            new_wires.push_back(_wires[idx]);
        }
        return WireSlot(new_slot_meta, new_wires);
    }
    /** range slicing*/
    WireSlot WireSlot::operator() (const std::string& start_field,
                        const std::string& end_field){
        int idx    = get_idx_ptr(start_field);
        int end_idx = get_idx_ptr(end_field) + 1;

        mf_assert(is_valid_idx(idx)   , "field name " + start_field + " not found");
        mf_assert(is_valid_idx(end_idx-1), "field name " + end_field + " not found");

        return (*this)(idx, end_idx);
    }

    /** individual slicing*/
    WireSlot WireSlot::operator() (const std::vector<int>& field_idxs){
        SlotMeta new_slot_meta = _meta(field_idxs);
        std::vector<Wire*> new_wires;
        for (auto idx: field_idxs){
            new_wires.push_back(_wires[idx]);
        }
        return WireSlot(new_slot_meta, new_wires);
    }

    WireSlot WireSlot::operator() (const std::vector<std::string>& field_names){
        std::vector<int> field_idxs = get_idx_ptrs(field_names);
        return (*this)(field_idxs);
    }

    WireSlot WireSlot::operator + (WireSlot& rhs){
        SlotMeta new_slot_meta = _meta + rhs._meta;
        std::vector<Wire*> new_wires;
        new_wires.insert(new_wires.end(), _wires.begin(), _wires.end());
        new_wires.insert(new_wires.end(), rhs._wires.begin(), rhs._wires.end());
        return WireSlot(new_slot_meta, new_wires);
    }

    void WireSlot::add_wire(const std::string& field_name, Wire& wire){
        mf_assert(!is_there_field(field_name), "field name " + field_name + "already_exist");
        _meta        .add_field({field_name, wire.get_operable_slice().get_size()});
        _wires       .push_back(&wire);
        _hwFieldMetas.push_back({&wire, &wire});
    }

    void WireSlot::add_wire(const std::string& field_name, int size){
        mf_assert(size > 0, "wire size must be positive");
        Wire& new_added_wire = m_opr_wire(field_name, size);
        add_wire(field_name, new_added_wire);
    }

    void WireSlot::add_wire(const std::string& field_name, Operable& value){
        add_wire_base(field_name, value);
    }

    void WireSlot::try_add_wire(const std::string& field_name, Operable& value){
        if(!is_there_field(field_name)){
            add_wire_base(field_name, value);
        }
    }

    void WireSlot::add_wire_base(const std::string& field_name, Operable& value){
        Wire& new_added_wire = m_opr_wire(field_name, value.get_operable_slice().get_size());
        AsmNode* new_assign_node = new_added_wire.generate_basic_node(value, new_added_wire.get_operable_slice(), ASM_DIRECT);
        new_assign_node->dry_assign();
        delete new_assign_node;
        add_wire(field_name, new_added_wire);
    }

    /**
     *  dynamic indexing
     */
     WireSlotDynSliceAgent WireSlot::operator[](Operable& required_idx){
        return WireSlotDynSliceAgent(*this, required_idx, false);
    }

    WireSlotDynSliceAgent WireSlot::operator[](const OH& required_oh_idx){
        return WireSlotDynSliceAgent(*this, required_oh_idx._idx, true);
    }

    void WireSlot::do_glob_asm(AsmNode* asm_node) {
        assert(asm_node != nullptr);
        ModelController* ctrl = get_controller_ptr();
        assert(ctrl != nullptr);
        ctrl->on_wire_update(
            asm_node,
            nullptr
        );
    }

    /** it will match by name*/
    WireSlot& WireSlot::operator <<= (const Slot& ) const{
        mf_assert(false, "wire slot not support <<= WireSlot::operator");
        assert(false);
        // return *this;
    }

    WireSlot& WireSlot::operator <<= (const WireSlot&){
        mf_assert(false, "wire slot not support <<= WireSlot::operator");
        assert(false);
        //return *this;
    }

    WireSlot& WireSlot::operator = (const  Slot& rhs){
        do_non_block_asm(rhs, std::vector<int>{}, ASM_DIRECT);
        return *this;
    }

    WireSlot& WireSlot::operator = (const WireSlot& rhs){
        do_non_block_asm(rhs, std::vector<int>{}, ASM_DIRECT);
        return *this;
    }

}