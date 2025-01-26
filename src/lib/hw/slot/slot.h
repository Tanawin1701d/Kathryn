//
// Created by tanawin on 16/1/2025.
//

#ifndef LIB_HW_SLOT_SLOT_H
#define LIB_HW_SLOT_SLOT_H

#include <utility>

#include "model/controller/controller.h"
#include "rowMeta.h"
#include "slotProxy.h"

namespace kathryn{


    struct SlotMeta{
        Operable*   opr = nullptr;
        Assignable* asb = nullptr;

    };

    class Slot: public Module{
    public:
        /**metadata zone*/
        RowMeta _meta;
        int     _ident;
        /**hardware zone*/
        std::vector<SlotMeta> hwMetas;

        explicit Slot(RowMeta  meta, int identVal):
        _meta(std::move(meta)),
        _ident(identVal){}

        [[nodiscard]]
        bool isContainIdx(int startIdx, int stopIdx)const{
            mfAssert(startIdx < stopIdx, "isContainIdx have invalid input");
            return _meta.isThereIdx(startIdx) && _meta.isThereIdx(stopIdx-1);
        }

        SlotOpr operator() (int startIdx, int stopIdx)const{

            mfAssert(isContainIdx(startIdx, stopIdx),
                "slice slot opr index out of range");

            std::vector<Operable*> repFields;
            for (int i = startIdx; i < stopIdx; i++){
                repFields.push_back(hwMetas[i].opr);
            }
            SlotOpr slicedOpr(_meta(startIdx, stopIdx), repFields);
            return slicedOpr;

        }


        /**meta-data getter system*/

        [[nodiscard]]
        SlotOpr getSlotOpr() const{
            return operator()(0, getAmtField());
        }

        [[nodiscard]]
        SlotOpr getSlotOpr(const RowMeta& rowMeta) const{
            std::vector<Operable*> oprs;
            for (const auto& field: rowMeta.getAllFields()){
                int idx = _meta.getFieldIdx(field._fieldName);
                oprs.push_back(hwMetas[idx].opr);
            }
            return SlotOpr(rowMeta, oprs);
        }

        [[nodiscard]]
        int getAmtField()const{return _meta.getSize();}

        [[nodiscard]]
        RowMeta getMeta()const{return _meta;}

        /**  assigning system*/

        /////////// main assigning
        void assignCore(const SlotOpr& rhsSlotOpr, int startIdx, int stopIdx, Operable& condition){

            int size = stopIdx - startIdx;
            mfAssert(size == rhsSlotOpr.getAmtField(), "size is not equal");
            mfAssert(startIdx >= 0 && stopIdx <= hwMetas.size(), "statIdx error");

            for(int i = 0; i < (stopIdx-startIdx); i++){
                FieldMeta lhsField = _meta.getField(startIdx+i);
                FieldMeta rhsField = rhsSlotOpr.getMeta().getField(i);
                mfAssert(lhsField.checkEqualField(rhsField),
                    "assign fieldNot equal fieldName lhs " + lhsField._fieldName + " rhs " + rhsField._fieldName);

                hwMetas[startIdx + i].asb->addUpdateMeta(new UpdateEvent({
                    &condition,
                    nullptr,
                    rhsSlotOpr._repFields[i],
                    hwMetas[startIdx + i].opr->getOperableSlice(),
                    DEFAULT_UE_PRI_USER
                }));
            }
        }

        void assignCore(const SlotOpr& rhsSlotOpr,
                        const std::string& startFieldName, const std::string& endFieldName,
                        Operable& condition){
            int startIdx = _meta.getFieldIdx(startFieldName);
            int stopIdx  = _meta.getFieldIdx(endFieldName);
            stopIdx++;
            assignCore(rhsSlotOpr, startIdx, stopIdx, condition);

        }

        void assignCore(const Slot& rhsSlot,
                         int startIdx, int stopIdx, int rhsStartIdx,
                         Operable& condition){
            assignCore(rhsSlot.getSlotOpr(), startIdx, stopIdx, condition);
        }

        Slot& assignCore(const Slot& rhsSlot,
                     const std::string& startFieldName, const std::string& endFieldName,
                     Operable& condition){ ///[startFieldName, endFieldName]
            ///// get the index and assign it
            int startIdx = _meta.getFieldIdx(startFieldName);
            int stopIdx  = _meta.getFieldIdx(endFieldName);
            stopIdx++;
            int rhsStartIdx = rhsSlot._meta.getFieldIdx(startFieldName);
            assignCore(rhsSlot, startIdx, stopIdx, rhsStartIdx, condition);

            return *this;
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
