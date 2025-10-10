//
// Created by tanawin on 18/9/25.
//

#include "regSlot.h"
#include "model/controller/controller.h"

namespace kathryn{

    //////// RegSlotDynSliceAgent

    RegSlotDynSliceAgent& RegSlotDynSliceAgent::operator <<=(Operable& rhsOpr){
        _masterSlot.doBlockAsm(rhsOpr, _requiredIdx, ASM_DIRECT);
        return *this;
    }

    RegSlotDynSliceAgent& RegSlotDynSliceAgent::operator <<=(ull rhsVal){
        _masterSlot.doBlockAsm(rhsVal, _requiredIdx, ASM_DIRECT);
        return *this;
    }

    RegSlotDynSliceAgent& RegSlotDynSliceAgent::operator =(Operable& rhsOpr){
        _masterSlot.doNonBlockAsm(rhsOpr, _requiredIdx, ASM_EQ_DEPNODE);
        return *this;
    }

    RegSlotDynSliceAgent& RegSlotDynSliceAgent::operator =(ull rhsVal){
        _masterSlot.doNonBlockAsm(rhsVal, _requiredIdx, ASM_EQ_DEPNODE);
        return *this;
    }


    /////// RegSlot


    RegSlot::RegSlot(const SlotMeta& slotMeta,
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

    RegSlot::RegSlot(const SlotMeta& slotMeta, const std::string& prefixName):
    Slot(slotMeta){
        /** this is used to initialize from slice*/
        initHwStructure(prefixName);
    }

    RegSlot::RegSlot(const std::vector<std::string>& fieldNames,
            const std::vector<int>&                  fieldSizes,
            const std::string&                       prefixName):
    Slot(fieldNames, fieldSizes){
        /** create new reg*/
        initHwStructure(prefixName);
    }

    void RegSlot::initHwStructure(const std::string& prefixName){
        for(int idx = 0; idx < _meta.getNumField(); idx++){
            FieldMeta fieldMeta = _meta.getCopyField(idx);
            mfAssert(fieldMeta._size > 0, "field " + fieldMeta._name + " is not pass integrity test");
            Reg* newReg = &mOprReg(prefixName +
                "colIdx_" + std::to_string(idx) +
                "_" + fieldMeta._name,
                fieldMeta._size);
            _regs.push_back(newReg);
            _hwFieldMetas.push_back({newReg, newReg});
        }
    }


    /**
     * The main function to overwrite the assignment
     */
    void RegSlot::doGlobAsm(
        const Slot& rhs,
        const std::vector<int>& srcMatchIdxs,
        const std::vector<int>& desMatchIdxs,
        const std::vector<int>& exceptIdxs,
        ASM_TYPE asmType) {

        AsmNode* asmNode = genGrpAsmNode(
            rhs,
            srcMatchIdxs,
            desMatchIdxs,
            exceptIdxs,
            asmType
        );
        doGlobAsm(asmNode);
    }

    void RegSlot::doGlobAsm(Operable& srcOpr,
                            Operable& requiredIdx,
                            ASM_TYPE asmType) {
        AsmNode* asmNode = genGrpAsmNode(srcOpr, requiredIdx, asmType);
        doGlobAsm(asmNode);
    }

    void RegSlot::doGlobAsm(AsmNode* asmNode) {
        assert(asmNode != nullptr);
        ModelController* ctrl = getControllerPtr();
        assert(ctrl != nullptr);
        ctrl->on_wire_update(
            asmNode,
            nullptr
        );
    }

    void RegSlot::makeResetEvent(ull resetValue){
        for (Reg* reg: _regs){
            reg->makeResetEvent(resetValue);
        }
    }

    void RegSlot::makeResetEvent(int colIdx, ull resetValue){
        mfAssert(isValidIdx(colIdx), "index out of range to get " + std::to_string(colIdx));
        _regs[colIdx]->makeResetEvent(resetValue);
    }

    void RegSlot::makeResetEvent(const std::string& colName, ull resetValue){
        int colIdx = getIdx(colName);
        makeResetEvent(colIdx, resetValue);
    }


    /***
     *  static slicing
     */

    /** single slicing*/
    Reg& RegSlot::operator () (int idx){
        mfAssert(isValidIdx(idx), "index out of range to get " + std::to_string(idx));
        return *_regs[idx];
    }

    Reg& RegSlot::operator () (const std::string& fieldName){
        int idx = getIdx(fieldName);
        mfAssert(_meta.isValidIdx(idx), "field name " + fieldName + " not found");
        return *_regs[idx];
    }

    RegSlot RegSlot::operator() (int start, int end){
        SlotMeta newSlotMeta = _meta(start, end);
        std::vector<Reg*> newRegs;
        for (int idx = start; idx < end; idx++){
            newRegs.push_back(_regs[idx]);
        }
        return RegSlot(newSlotMeta, newRegs);
    }

    /** range slicing*/
    RegSlot RegSlot::operator() (const std::string& startField,
                        const std::string& endField){
        int idx    = getIdx(startField);
        int endIdx = getIdx(endField) + 1;

        mfAssert(isValidIdx(idx)   , "field name " + startField + " not found");
        mfAssert(isValidIdx(endIdx), "field name " + endField + " not found");

        return (*this)(idx, endIdx);
    }

    /** individual slicing*/
    RegSlot RegSlot::operator() (const std::vector<int>& fieldIdxs){
        SlotMeta newSlotMeta = _meta(fieldIdxs);
        std::vector<Reg*> newRegs;
        for (auto idx: fieldIdxs){
            newRegs.push_back(_regs[idx]);
        }
        return RegSlot(newSlotMeta, newRegs);
    }

    RegSlot RegSlot::operator() (const std::vector<std::string>& fieldNames){
        std::vector<int> fieldIdxs = getIdxs(fieldNames);
        return (*this)(fieldIdxs);
    }

    RegSlot RegSlot::operator + (const RegSlot& rhs){
        SlotMeta newSlotMeta = getMeta() + rhs.getMeta();
        std::vector<Reg*> newRegs = _regs;
        _regs.insert(_regs.end(), rhs._regs.begin(), rhs._regs.end());
        return {newSlotMeta, newRegs};
    }


    /**
     *  dynamic indexing
     */
    RegSlotDynSliceAgent RegSlot::operator[](Operable& requiredIdx){
        return {*this, requiredIdx};
    }

    /** it will match by name*/
    RegSlot& RegSlot::operator <<= (const Slot& rhs){
        doBlockAsm(rhs, ASM_DIRECT);
        return *this;
    }

    RegSlot& RegSlot::operator <<= (const RegSlot& rhs){
        doBlockAsm(rhs, ASM_DIRECT);
        return *this;
    }

    RegSlot& RegSlot::operator = (const Slot& rhs){
        doNonBlockAsm(rhs, ASM_EQ_DEPNODE);
        return *this;
    }

    RegSlot& RegSlot::operator = (const RegSlot& rhs){
        doNonBlockAsm(rhs, ASM_EQ_DEPNODE);
        return *this;
    }





}