//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_WIRESLOT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_WIRESLOT_H


#include "slot.h"
#include "model/hw_component/wire/wire.h"

namespace kathryn{
    class WireSlotDynSliceAgent : public SlotDynSliceAgent{
    public:
        WireSlotDynSliceAgent(
            Slot& slot,
            Operable& required_idx,
            bool is_oh
        ) : SlotDynSliceAgent(slot, required_idx, is_oh){}

        WireSlotDynSliceAgent& operator <<=(Operable& rhs_opr);
        WireSlotDynSliceAgent& operator <<=(ull       rhs_val);

        WireSlotDynSliceAgent& operator =(Operable& rhs_opr);
        WireSlotDynSliceAgent& operator =(ull       rhs_val);


    };

    class WireSlot: public Slot{

    protected:
        std::vector<Wire*> _wires;

        void add_wire_base(const std::string& field_name, Operable& value);

    public:
        WireSlot(const SlotMeta& slot_meta,
                const std::vector<Wire*>& wires
        );

        explicit WireSlot(const SlotMeta& slot_meta, const std::string& prefix_name = "WireSlot");

        WireSlot(const std::vector<std::string>& field_names,
                 const std::vector<int>&         field_sizes,
                 const std::string&              prefix_name = "WireSlot");

        explicit WireSlot(const Slot& rhs, const std::string& prefix_name = "WireSlot");

        ~WireSlot() override = default;

        void init_hw_structure(const std::string& prefix_name) override;
        /**
         * The main function to overwrite the assignment
         */
        void do_glob_asm(
            const Slot& rhs,
            const std::vector<int>& src_match_idxs,
            const std::vector<int>& des_match_idxs,
            const std::vector<int>& except_idxs,
            ASM_TYPE asm_type) override;

        void do_glob_asm(Operable& src_opr,
                       Operable& required_idx,
                       ASM_TYPE  asm_type,
                       bool      is_oh) override;

        void do_glob_asm(AsmNode* asm_node) override;


        /***
         *  static slicing
         */

        /** single slicing*/
        Wire& operator () (int idx);

        Wire& operator () (const std::string& field_name);

        WireSlot operator() (int start, int end);
        /** range slicing*/
        WireSlot operator() (const std::string& start_field,
                            const std::string& end_field);

        /** individual slicing*/
        WireSlot operator() (const std::vector<int>& field_idxs);

        WireSlot operator() (const std::vector<std::string>& field_names);

        WireSlot operator + (WireSlot& rhs);

        void add_wire(const std::string& field_name, Wire& value);
        void add_wire(const std::string& field_name, int size);
        void add_wire(const std::string& field_name, Operable& value);
        void try_add_wire(const std::string& field_name, Operable& value); //// if there IS exist wire skip it

        /**
         *  dynamic indexing
         */
        WireSlotDynSliceAgent operator[](Operable& required_idx);
        WireSlotDynSliceAgent operator[](const OH& required_oh_idx);


        /** it will match by name*/
        WireSlot& operator <<= (const Slot& rhs) const;
        WireSlot& operator <<= (const WireSlot& rhs);

        WireSlot& operator = (const Slot& rhs);
        WireSlot& operator = (const WireSlot& rhs);



    };
}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_WIRESLOT_H
