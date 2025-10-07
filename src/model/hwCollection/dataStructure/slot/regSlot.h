//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_REGSLOT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_REGSLOT_H


#include "slot.h"
#include "model/hwComponent/register/register.h"

namespace kathryn{

    class RegSlotDynSliceAgent : public SlotDynSliceAgent{
    public:
        RegSlotDynSliceAgent(
            Slot& slot,
            Operable& requiredIdx
        ) : SlotDynSliceAgent(slot, requiredIdx){}

        RegSlotDynSliceAgent& operator <<=(Operable& rhsOpr);
        RegSlotDynSliceAgent& operator <<=(ull       rhsVal);

        RegSlotDynSliceAgent& operator =(Operable& rhsOpr);
        RegSlotDynSliceAgent& operator =(ull       rhsOpr);
    };

    class RegSlot: public Slot{

    protected:
        std::vector<Reg*> _regs;

    public:

        RegSlot() : Slot(){}

        RegSlot(const SlotMeta& slotMeta,
                const std::vector<Reg*>& regs
        );

        explicit RegSlot(const SlotMeta& slotMeta, const std::string& prefixName = "RegSlot" );

        RegSlot(const std::vector<std::string>& fieldNames,
                const std::vector<int>&         fieldSizes,
                const std::string& prefixName = "RegSlot");

        ~RegSlot() override = default;

        void initHwStructure(const std::string& prefixName) override;

        /**
         * The main function to overwrite the assignment
         */
        void doGlobAsm(
            const Slot& rhs,
            const std::vector<int>& srcMatchIdxs,
            const std::vector<int>& desMatchIdxs,
            const std::vector<int>& exceptIdxs,
            ASM_TYPE asmType) override;

        void doGlobAsm(Operable& srcOpr,
                       Operable& requiredIdx,
                       ASM_TYPE asmType) override;

        void doGlobAsm(AsmNode* asmNode) override;

        void makeResetEvent(ull resetValue = 0);

        /***
         *  static slicing
         */

        /** single slicing*/
        Reg& operator () (int idx);
        Reg& operator () (const std::string& fieldName);

        RegSlot operator() (int start, int end);
        /** range slicing*/
        RegSlot operator() (const std::string& startField,
                            const std::string& endField);

        /** individual slicing*/
        RegSlot operator() (const std::vector<int>& fieldIdxs);

        RegSlot operator() (const std::vector<std::string>& fieldNames);

        RegSlot operator + (const RegSlot& rhs);

        /**
         *  dynamic indexing
         */
        RegSlotDynSliceAgent operator[](Operable& requiredIdx);


        /** it will match by name*/
        RegSlot& operator <<= (const Slot& rhs);
        RegSlot& operator <<= (const RegSlot& rhs);

        RegSlot& operator = (const Slot& rhs);
        RegSlot& operator = (const RegSlot& rhs);

    };
}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_REGSLOT_H
