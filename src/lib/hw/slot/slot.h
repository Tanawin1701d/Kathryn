//
// Created by tanawin on 16/1/2025.
//

#ifndef LIB_HW_SLOT_SLOT_H
#define LIB_HW_SLOT_SLOT_H

#include <utility>

#include "model/controller/controller.h"
#include "rowMeta.h"

namespace kathryn{


    struct SlotMeta{
        Operable*   opr = nullptr;
        Assignable* asb = nullptr;

    };

    class Slot: public Module{
    public:
        /**metadata zone*/
        bool    _readOnly = false; ///// in case there is no assigable in hwMetaas
        RowMeta _meta;
        int     _ident;
        /**hardware zone*/
        std::vector<SlotMeta> hwMetas;

        explicit Slot(RowMeta  meta, int identVal):
        _meta(std::move(meta)),
        _ident(identVal){}

        explicit Slot(RowMeta meta, int identVal, const std::vector<SlotMeta>& slotMetas):
        _meta(std::move(meta)),
        _ident(identVal),
        hwMetas(slotMetas){
            mfAssert(!slotMetas.empty(), "cannot create slot with empt slot Metas");
            if (slotMetas[0].asb == nullptr){_readOnly = true;}
            integrityCheck();
        }

        // explicit Slot(RowMeta meta, int identVal, const std::vector<Operable*>& oprMetas):
        // _meta(std::move(meta)),
        // _ident(identVal){
        //     for (Operable* opr: oprMetas){
        //         mfAssert(opr != nullptr, "buildReadOnly slot cannot have opr as nullptr");
        //         hwMetas.push_back({opr, nullptr});
        //     }
        //     integrityCheck();
        // }

        /** checking idx*/
        [[nodiscard]]
        bool isContainIdx(int startIdx, int stopIdx)const{
            mfAssert(startIdx < stopIdx, "isContainIdx have invalid input");
            return _meta.isThereIdx(startIdx) && _meta.isThereIdx(stopIdx-1);
        }
        bool integrityCheck(){
            if (hwMetas.size() != _meta.getSize()){return false;}
            for (int idx = 0; idx < _meta.getSize(); idx++){
                if (_meta.getField(idx)._fieldSize != hwMetas[idx].opr->getOperableSlice().getSize()){
                    return false;
                }
            }
            return true;
        }

        Slot operator() (int startIdx, int stopIdx)const{

            mfAssert(isContainIdx(startIdx, stopIdx),
                "slice slot opr index out of range");

            std::vector<SlotMeta> slotMetas;
            for (int i = startIdx; i < stopIdx; i++){
                slotMetas.push_back(hwMetas[i]);
            }
            Slot slicedSlot(_meta(startIdx, stopIdx), _ident, slotMetas);
            return slicedSlot;

        }

        Slot operator() (const std::string& startName, const std::string& stopName) const{
            int startIdx = _meta.getFieldIdx(startName);
            int stopIdx  = _meta.getFieldIdx(stopName) + 1;
            return operator()(startIdx, stopIdx);
        }

        Slot operator() (const std::vector<std::string>& fieldNames){
            std::vector<FieldMeta> fms;
            std::vector<SlotMeta> slotMetas;
            for (auto& fieldName: fieldNames){
                int idxInCurSlot = _meta.getFieldIdx(fieldName);
                fms.push_back(_meta.getField(fieldName));
                slotMetas.push_back(hwMetas[idxInCurSlot]);
            }
            return Slot(RowMeta(fms), _ident, slotMetas);
        }

        Slot operator() (const RowMeta& rowMeta){
            return operator() (rowMeta.getAllFieldNames());
        }

        Slot operator+ (const Slot& rhs){
            ////// todo pool slot
            return Slot();
        }

        void setNewRowMeta(const RowMeta& newRowMeta){
            ///// todo new rowMeta
        }

        [[nodiscard]]
        int getAmtField()const{return _meta.getSize();}

        [[nodiscard]]
        RowMeta getMeta()const{return _meta;}

        /**  assigning system*/

        /////////// main assigning
        void assignCore(const Slot& rhsSlot, Operable& condition){

            int srcIdx = 0;
            for (const FieldMeta& fm: rhsSlot.getMeta().getAllFields()){
                int desIdx = _meta.getFieldIdx(fm._fieldName);
                mfAssert(isContainIdx(desIdx, desIdx+1), "cannot assign fieldName: " + fm._fieldName);
                hwMetas[desIdx].asb->addUpdateMeta(new UpdateEvent({
                    &condition,
                    nullptr,
                    rhsSlot.hwMetas[srcIdx].opr,
                    hwMetas[srcIdx].opr->getOperableSlice(),
                    DEFAULT_UE_PRI_USER
                }));
                srcIdx += 1;
            }
        }

        void assignCore(const std::string& startFieldName,
                        Operable& dayta,
                        Operable& condition){
            RowMeta rm({startFieldName}, {dayta.getOperableSlice().getSize()});
            Slot rhsSlot(rm, -1, {{&dayta, nullptr}});
            assignCore(rhsSlot, condition);
        }

        /** hardware getter system*/
        SlotMeta getHwSlotMeta(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return getHwSlotMeta(idx);
        }
        SlotMeta getHwSlotMeta(int idx){
            mfAssert(isContainIdx(idx, idx+1), "cannot find fieldIdx " + std::to_string(idx));
            return hwMetas[idx];
        }

        Operable& getField(const std::string& fieldName){
            return *getHwSlotMeta(fieldName).opr;
        }



    };

    class RegSlot: public Slot{
    public:
        std::vector<Reg*> hwFields;

        ///////// constructor
        explicit RegSlot(const RowMeta& meta, int identVal):
        Slot(meta,identVal){
            for (const auto& field: meta.getAllFields()){
                hwFields.push_back(&mOprReg(field._fieldName + "_ident_" + std::to_string(_ident), field._fieldSize));
                hwMetas .push_back({*hwFields.rbegin(),*hwFields.rbegin()});
            }
        }
        /** hardware getter system*/
        Reg& get(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return get(idx);
        }
        Reg& get(int idx){
            mfAssert(idx >= 0 && idx < hwFields.size(), "cannot find fieldIdx " + std::to_string(idx));
            return *hwFields[idx];
        }
    };

    class WireSlot: public Slot{
    public:
        std::vector<Wire*> hwFields;

        explicit WireSlot(const RowMeta& meta, int identVal):
        Slot(meta,identVal){
            for (const auto& field: meta.getAllFields()){
                hwFields.push_back(&mOprWire(field._fieldName + "_ident_" + std::to_string(_ident), field._fieldSize));
                hwMetas .push_back({*hwFields.rbegin(),*hwFields.rbegin()});
            }
        }

        /** hardware getter system*/

        Wire& get(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return get(idx);
        }

        Wire& get(int idx){
            mfAssert(idx >= 0 && idx < hwFields.size(), "cannot find fieldIdx " + std::to_string(idx));
            return *hwFields[idx];
        }
    };

}

#endif //LIB_HW_SLOT_SLOT_H
