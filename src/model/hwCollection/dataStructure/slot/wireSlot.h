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

        WireSlotDynSliceAgent& operator <<=(Operable& rhsOpr){
            mfAssert(false, "wire slot not support <<= operator");
            return *this;
        }

        WireSlotDynSliceAgent& operator =(Operable& rhsOpr){
            _masterSlot.doNonBlockAsm(rhsOpr, _requiredIdx, ASM_DIRECT);
            return *this;
        }


    };

    class WireSlot: public Slot{

    protected:
        std::vector<Wire*> _wires;

    public:
        WireSlot(const SlotMeta& slotMeta,
                const std::vector<Wire*>& wires
        ):
        Slot(slotMeta){
            /** this is used to initialize from slice*/
            mfAssert(slotMeta.getNumField() == wires.size(), "field_metas size not match with wires size");
            _wires = wires;
            for(Wire* wire: _wires){
                _hwFieldMetas.push_back({wire, wire});
            }
        }

        WireSlot(const SlotMeta& slotMeta):
        Slot(slotMeta){
            /** this is used to initialize from slice*/
            for(int idx = 0; idx < slotMeta.getNumField(); idx++){
                FieldMeta fieldMeta = slotMeta.getCopyField(idx);
                mfAssert(fieldMeta._size > 0, "field " + fieldMeta._name + " is not pass integrity test");
                Wire* newWire = &makeOprWire(fieldMeta._name, fieldMeta._size);
                _wires.push_back(newWire);
                _hwFieldMetas.push_back({newWire, newWire});
            }
        }

        WireSlot(const std::vector<std::string>& fieldNames,
                const std::vector<int>&         fieldSizes):
        Slot(fieldNames, fieldSizes){
            std::vector<HwFieldMeta> hwFieldMetas;
            /** create new wire*/
            int idx = 0;
            for(idx = 0; idx < fieldNames.size(); idx++){
                Wire* newWire = &makeOprWire(fieldNames[idx], fieldSizes[idx]);
                _wires.push_back(newWire);
                _hwFieldMetas.push_back({newWire, newWire});
            }
        }


        /***
         *  static slicing
         */

        /** single slicing*/
        Wire& operator () (int idx){
            mfAssert(isValidIdx(idx), "index out of range to get " + std::to_string(idx));
            return *_wires[idx];
        }

        Wire& operator () (const std::string& fieldName){
            int idx = getIdx(fieldName);
            mfAssert(_meta.isValidIdx(idx), "field name " + fieldName + " not found");
            return *_wires[idx];
        }

        WireSlot operator() (int start, int end){
            SlotMeta newSlotMeta = _meta(start, end);
            std::vector<Wire*> newWires;
            for (int idx = start; idx < end; idx++){
                newWires.push_back(_wires[idx]);
            }
            return WireSlot(newSlotMeta, newWires);
        }
        /** range slicing*/
        WireSlot operator() (const std::string& startField,
                            const std::string& endField){
            int idx    = getIdx(startField);
            int endIdx = getIdx(endField) + 1;

            mfAssert(isValidIdx(idx)   , "field name " + startField + " not found");
            mfAssert(isValidIdx(endIdx), "field name " + endField + " not found");

            return (*this)(idx, endIdx);
        }

        /** individual slicing*/
        WireSlot operator() (const std::vector<int>& fieldIdxs){
            SlotMeta newSlotMeta = _meta(fieldIdxs);
            std::vector<Wire*> newWires;
            for (auto idx: fieldIdxs){
                newWires.push_back(_wires[idx]);
            }
            return WireSlot(newSlotMeta, newWires);
        }

        WireSlot operator() (const std::vector<std::string>& fieldNames){
            std::vector<int> fieldIdxs = getIdxs(fieldNames);
            return (*this)(fieldIdxs);
        }

        /**
         *  dynamic indexing
         */
         WireSlotDynSliceAgent operator[](Operable& requiredIdx){
            
            return WireSlotDynSliceAgent(*this, requiredIdx);
        }

        /**
         * The main function to overwrite the assignment
         */
        void doGlobAsm(
            Slot& rhs,
            const std::vector<int>& srcMatchIdxs,
            const std::vector<int>& desMatchIdxs,
            const std::vector<int>& exceptIdxs,
            ASM_TYPE asmType) override{

            AsmNode* asmNode = genGrpAsmNode(
                               rhs,
                               srcMatchIdxs,
                               desMatchIdxs,
                               exceptIdxs,
                               asmType
                               );
            doGlobAsm(asmNode);
        }

        void doGlobAsm(Operable& srcOpr,
                       Operable& requiredIdx,
                       ASM_TYPE asmType) override{
            AsmNode* asmNode = genGrpAsmNode(srcOpr, requiredIdx, asmType);
            doGlobAsm(asmNode);
        }

        void doGlobAsm(AsmNode* asmNode) override{
            assert(asmNode != nullptr);
            ModelController* ctrl = getControllerPtr();
            assert(ctrl != nullptr);
            ctrl->on_wire_update(
                asmNode,
                nullptr
            );
        }

        /** it will match by name*/
        WireSlot& operator <<= (Slot& rhs){
            mfAssert(false, "wire slot not support <<= operator");
            return *this;
        }

        WireSlot& operator = (Slot& rhs){
            doNonBlockAsm(rhs, std::vector<int>{}, ASM_DIRECT);
            return *this;
        }

    };
}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_WIRESLOT_H
