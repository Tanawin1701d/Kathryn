//
// Created by tanawin on 18/9/25.
//

#include "wireSlot.h"
#include "model/controller/controller.h"

namespace kathryn{

    WireSlotDynSliceAgent& WireSlotDynSliceAgent::operator <<=(Operable& rhsOpr){
        mfAssert(false, "wire slot not support <<= operator");
        return *this;
    }

    WireSlotDynSliceAgent& WireSlotDynSliceAgent::operator <<=(ull rhsVal){
        mfAssert(false, "wire slot not support <<= operator");
        return *this;
    }

    WireSlotDynSliceAgent& WireSlotDynSliceAgent::operator =(Operable& rhsOpr){
        _masterSlot.doNonBlockAsm(rhsOpr, _requiredIdx, ASM_DIRECT);
        return *this;
    }

    WireSlotDynSliceAgent& WireSlotDynSliceAgent::operator =(ull rhsVal){
        _masterSlot.doNonBlockAsm(rhsVal, _requiredIdx, ASM_DIRECT);
        return *this;
    }

    WireSlot::WireSlot(const SlotMeta& slotMeta,
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

    WireSlot::WireSlot(const SlotMeta& slotMeta, const std::string& prefixName):
        Slot(slotMeta){
            /** this is used to initialize from slice*/
            initHwStructure(prefixName);
        }

    WireSlot::WireSlot(const std::vector<std::string>& fieldNames,
                const std::vector<int>&         fieldSizes,
                const std::string&              prefixName):
        Slot(fieldNames, fieldSizes){
            initHwStructure(prefixName);
    }

    WireSlot::WireSlot(const Slot& rhs, const std::string& prefixName):
    Slot(rhs){
        initHwStructure(prefixName);
        AsmNode* asmNode = genGrpAsmNode(rhs, ASM_DIRECT);
        asmNode->dryAssign();
        delete asmNode;
    }

    void WireSlot::initHwStructure(const std::string& prefixName){

        for(int idx = 0; idx < _meta.getNumField(); idx++){
            FieldMeta fieldMeta = _meta.getCopyField(idx);
            mfAssert(fieldMeta._size > 0, "field " + fieldMeta._name + " is not pass integrity test");
            Wire* newWire = &mOprWire(prefixName + "_" +fieldMeta._name, fieldMeta._size);
            _wires.push_back(newWire);
            _hwFieldMetas.push_back({newWire, newWire});
        }

    }

        /**
             * The main function to overwrite the assignment
             */
    void WireSlot::doGlobAsm(
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

    void WireSlot::doGlobAsm(Operable& srcOpr,
                   Operable& requiredIdx,
                   ASM_TYPE asmType) {
        AsmNode* asmNode = genGrpAsmNode(srcOpr, requiredIdx, asmType);
        doGlobAsm(asmNode);
    }


    /***
     *  static slicing
     */

    /** single slicing*/
    Wire& WireSlot::operator () (int idx){
        mfAssert(isValidIdx(idx), "index out of range to get " + std::to_string(idx));
        return *_wires[idx];
    }

    Wire& WireSlot::operator () (const std::string& fieldName){
        int idx = getIdx(fieldName);
        mfAssert(_meta.isValidIdx(idx), "field name " + fieldName + " not found");
        return *_wires[idx];
    }

    WireSlot WireSlot::operator() (int start, int end){
        SlotMeta newSlotMeta = _meta(start, end);
        std::vector<Wire*> newWires;
        for (int idx = start; idx < end; idx++){
            newWires.push_back(_wires[idx]);
        }
        return WireSlot(newSlotMeta, newWires);
    }
    /** range slicing*/
    WireSlot WireSlot::operator() (const std::string& startField,
                        const std::string& endField){
        int idx    = getIdx(startField);
        int endIdx = getIdx(endField) + 1;

        mfAssert(isValidIdx(idx)   , "field name " + startField + " not found");
        mfAssert(isValidIdx(endIdx), "field name " + endField + " not found");

        return (*this)(idx, endIdx);
    }

    /** individual slicing*/
    WireSlot WireSlot::operator() (const std::vector<int>& fieldIdxs){
        SlotMeta newSlotMeta = _meta(fieldIdxs);
        std::vector<Wire*> newWires;
        for (auto idx: fieldIdxs){
            newWires.push_back(_wires[idx]);
        }
        return WireSlot(newSlotMeta, newWires);
    }

    WireSlot WireSlot::operator() (const std::vector<std::string>& fieldNames){
        std::vector<int> fieldIdxs = getIdxs(fieldNames);
        return (*this)(fieldIdxs);
    }

    WireSlot WireSlot::operator + (WireSlot& rhs){
        SlotMeta newSlotMeta = _meta + rhs._meta;
        std::vector<Wire*> newWires;
        newWires.insert(newWires.end(), _wires.begin(), _wires.end());
        newWires.insert(newWires.end(), rhs._wires.begin(), rhs._wires.end());
        return WireSlot(newSlotMeta, newWires);
    }

    /**
     *  dynamic indexing
     */
     WireSlotDynSliceAgent WireSlot::operator[](Operable& requiredIdx){

        return WireSlotDynSliceAgent(*this, requiredIdx);
    }

    void WireSlot::doGlobAsm(AsmNode* asmNode) {
        assert(asmNode != nullptr);
        ModelController* ctrl = getControllerPtr();
        assert(ctrl != nullptr);
        ctrl->on_wire_update(
            asmNode,
            nullptr
        );
    }

    /** it will match by name*/
    WireSlot& WireSlot::operator <<= (Slot& rhs){
        mfAssert(false, "wire slot not support <<= WireSlot::operator");
        return *this;
    }

    WireSlot& WireSlot::operator = (Slot& rhs){
        doNonBlockAsm(rhs, std::vector<int>{}, ASM_DIRECT);
        return *this;
    }

}