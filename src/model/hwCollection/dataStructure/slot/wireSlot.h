//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_WIRESLOT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_WIRESLOT_H


#include "slot.h"
#include "model/hwComponent/wire/wire.h"

namespace kathryn{
    class WireSlotDynSliceAgent : public SlotDynSliceAgent{
    public:
        WireSlotDynSliceAgent(
            Slot& slot,
            Operable& requiredIdx
        ) : SlotDynSliceAgent(slot, requiredIdx){}

        WireSlotDynSliceAgent& operator <<=(Operable& rhsOpr);
        WireSlotDynSliceAgent& operator <<=(ull       rhsVal);

        WireSlotDynSliceAgent& operator =(Operable& rhsOpr);
        WireSlotDynSliceAgent& operator =(ull       rhsVal);


    };

    class WireSlot: public Slot{

    protected:
        std::vector<Wire*> _wires;

        void addWireBase(const std::string& fieldName, Operable& value);

    public:
        WireSlot(const SlotMeta& slotMeta,
                const std::vector<Wire*>& wires
        );

        explicit WireSlot(const SlotMeta& slotMeta, const std::string& prefixName = "WireSlot");

        WireSlot(const std::vector<std::string>& fieldNames,
                 const std::vector<int>&         fieldSizes,
                 const std::string&              prefixName = "WireSlot");

        explicit WireSlot(const Slot& rhs, const std::string& prefixName = "WireSlot");

        ~WireSlot() override = default;

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


        /***
         *  static slicing
         */

        /** single slicing*/
        Wire& operator () (int idx);

        Wire& operator () (const std::string& fieldName);

        WireSlot operator() (int start, int end);
        /** range slicing*/
        WireSlot operator() (const std::string& startField,
                            const std::string& endField);

        /** individual slicing*/
        WireSlot operator() (const std::vector<int>& fieldIdxs);

        WireSlot operator() (const std::vector<std::string>& fieldNames);

        WireSlot operator + (WireSlot& rhs);

        void addWire(const std::string& fieldName, Wire& value);
        void addWire(const std::string& fieldName, int size);
        void addWire(const std::string& fieldName, Operable& value);
        void tryAddWire(const std::string& fieldName, Operable& value); //// if there IS exist wire skip it

        /**
         *  dynamic indexing
         */
         WireSlotDynSliceAgent operator[](Operable& requiredIdx);


        /** it will match by name*/
        WireSlot& operator <<= (const Slot& rhs);
        WireSlot& operator <<= (const WireSlot& rhs);

        WireSlot& operator = (const Slot& rhs);
        WireSlot& operator = (const WireSlot& rhs);



    };
}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_WIRESLOT_H
