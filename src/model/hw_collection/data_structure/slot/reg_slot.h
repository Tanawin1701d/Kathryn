//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_REGSLOT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_REGSLOT_H


#include "slot.h"
#include "model/hw_component/register/register.h"

namespace kathryn{

    class RegSlotDynSliceAgent : public SlotDynSliceAgent{
    public:
        RegSlotDynSliceAgent(
            Slot& slot,
            Operable& required_idx,
            bool is_oh
        ) : SlotDynSliceAgent(slot, required_idx, is_oh){}

        RegSlotDynSliceAgent& operator <<=(Operable& rhs_opr);
        RegSlotDynSliceAgent& operator <<=(ull       rhs_val);

        RegSlotDynSliceAgent& operator =(Operable& rhs_opr);
        RegSlotDynSliceAgent& operator =(ull       rhs_opr);
    };

    class RegSlot: public Slot{

    protected:
        std::vector<Reg*> _regs;

    public:

        RegSlot() : Slot(){}

        RegSlot(const SlotMeta& slot_meta,
                const std::vector<Reg*>& regs
        );

        explicit RegSlot(const SlotMeta& slot_meta, const std::string& prefix_name = "RegSlot" );

        RegSlot(const std::vector<std::string>& field_names,
                const std::vector<int>&         field_sizes,
                const std::string& prefix_name = "RegSlot");

        ~RegSlot() override = default;

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

        void make_reset_event(ull reset_value = 0, CLOCK_MODE cm = CM_POSEDGE);
        void make_reset_event(int col_idx, ull reset_value, CLOCK_MODE cm = CM_POSEDGE);
        void make_reset_event(const std::string& col_name, ull reset_value, CLOCK_MODE cm = CM_POSEDGE);

        /***
         *  static slicing
         */

        /** single slicing*/
        Reg& operator () (int idx);
        Reg& operator () (const std::string& field_name);

        RegSlot operator() (int start, int end);
        /** range slicing*/
        RegSlot operator() (const std::string& start_field,
                            const std::string& end_field);

        /** individual slicing*/
        RegSlot operator() (const std::vector<int>& field_idxs);

        RegSlot operator() (const std::vector<std::string>& field_names);

        RegSlot operator + (const RegSlot& rhs);

        /**
         *  dynamic indexing
         */
        RegSlotDynSliceAgent operator[](Operable& required_idx);
        RegSlotDynSliceAgent operator[](const OH& required_oh_idx);

        /** it will match by name*/
        RegSlot& operator <<= (const Slot& rhs);
        RegSlot& operator <<= (const RegSlot& rhs);

        RegSlot& operator = (const Slot& rhs);
        RegSlot& operator = (const RegSlot& rhs);

    };
}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_REGSLOT_H
