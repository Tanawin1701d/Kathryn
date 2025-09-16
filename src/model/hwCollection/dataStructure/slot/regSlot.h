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

        RegSlotDynSliceAgent& operator <<=(Operable& rhsOpr){
            _masterSlot.doBlockAsm(rhsOpr, _requiredIdx, ASM_DIRECT);
            return *this;
        }

        RegSlotDynSliceAgent& operator =(Operable& rhsOpr){
            _masterSlot.doNonBlockAsm(rhsOpr, _requiredIdx, ASM_EQ_DEPNODE);
            return *this;
        }
    };

    class RegSlot: public Slot{

    protected:
        std::vector<Reg*> _regs;

    public:

        RegSlot(const SlotMeta& slotMeta,
                const std::vector<Reg*>& regs
        ):
        Slot(slotMeta){
            /** this is used to initialize from RegSlot slice*/
            mfAssert(slotMeta.getNumField() == regs.size(), "field_metas size not match with regs size");
            _regs = regs;
            for(Reg* reg: _regs){
                _hwFieldMetas.push_back({reg, reg});
            }
        }

        RegSlot(const SlotMeta& slotMeta):
        Slot(slotMeta){
            /** this is used to initialize from slice*/
            for(int idx = 0; idx < slotMeta.getNumField(); idx++){
                FieldMeta fieldMeta = slotMeta.getCopyField(idx);
                mfAssert(fieldMeta._size > 0, "field " + fieldMeta._name + " is not pass integrity test");
                Reg* newReg = &makeOprReg(fieldMeta._name, fieldMeta._size);
                _regs.push_back(newReg);
                _hwFieldMetas.push_back({newReg, newReg});
            }
        }

        RegSlot(const std::vector<std::string>& fieldNames,
                const std::vector<int>&         fieldSizes):
        Slot(fieldNames, fieldSizes){
            std::vector<HwFieldMeta> hwFieldMetas;
            /** create new reg*/
            int idx = 0;
            for(idx = 0; idx < fieldNames.size(); idx++){
                Reg* newReg = &makeOprReg(fieldNames[idx], fieldSizes[idx]);
                _regs.push_back(newReg);
                _hwFieldMetas.push_back({newReg, newReg});
            }
        }

        ~RegSlot() override{
            for(Reg* reg: _regs){
                delete reg;
            }
        };

        /***
         *  static slicing
         */

        /** single slicing*/
        Reg& operator () (int idx){
            mfAssert(isValidIdx(idx), "index out of range to get " + std::to_string(idx));
            return *_regs[idx];
        }

        Reg& operator () (const std::string& fieldName){
            int idx = getIdx(fieldName);
            mfAssert(_meta.isValidIdx(idx), "field name " + fieldName + " not found");
            return *_regs[idx];
        }

        RegSlot operator() (int start, int end){
            SlotMeta newSlotMeta = _meta(start, end);
            std::vector<Reg*> newRegs;
            for (int idx = start; idx < end; idx++){
                newRegs.push_back(_regs[idx]);
            }
            return RegSlot(newSlotMeta, newRegs);
        }
        /** range slicing*/
        RegSlot operator() (const std::string& startField,
                            const std::string& endField){
            int idx    = getIdx(startField);
            int endIdx = getIdx(endField) + 1;

            mfAssert(isValidIdx(idx)   , "field name " + startField + " not found");
            mfAssert(isValidIdx(endIdx), "field name " + endField + " not found");

            return (*this)(idx, endIdx);
        }

        /** individual slicing*/
        RegSlot operator() (const std::vector<int>& fieldIdxs){
            SlotMeta newSlotMeta = _meta(fieldIdxs);
            std::vector<Reg*> newRegs;
            for (auto idx: fieldIdxs){
                newRegs.push_back(_regs[idx]);
            }
            return RegSlot(newSlotMeta, newRegs);
        }

        RegSlot operator() (const std::vector<std::string>& fieldNames){
            std::vector<int> fieldIdxs = getIdxs(fieldNames);
            return (*this)(fieldIdxs);
        }

        RegSlot operator + (const RegSlot& rhs){
            SlotMeta newSlotMeta = getMeta() + rhs.getMeta();
            std::vector<Reg*> newRegs = _regs;
            _regs.insert(_regs.end(), rhs._regs.begin(), rhs._regs.end());
            return RegSlot(newSlotMeta, newRegs);
        }

        /**
         *  dynamic indexing
         */
        RegSlotDynSliceAgent operator[](Operable& requiredIdx){
            return RegSlotDynSliceAgent(*this, requiredIdx);
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
        RegSlot& operator <<= (Slot& rhs){
            doBlockAsm(rhs, ASM_DIRECT);
            return *this;
        }

        RegSlot& operator = (Slot& rhs){
            auto [srcMatchIdxs, desMatchIdxs] = matchByName(rhs);
            doNonBlockAsm(rhs, std::vector<int>{}, ASM_EQ_DEPNODE);
            return *this;
        }

    };
}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_REGSLOT_H
