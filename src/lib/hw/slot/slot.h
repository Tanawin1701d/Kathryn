//
// Created by tanawin on 16/1/2025.
//

#ifndef LIB_HW_SLOT_SLOT_H
#define LIB_HW_SLOT_SLOT_H

#include "model/controller/controller.h"
#include "rowMeta.h"
#include "slotProxy.h"

namespace kathryn{



    class Slot: public Module{
    public:
        /**metadata zone*/
        RowMeta _meta;
        int     _ident;
        /**hardware zone*/
        std::vector<Reg*> hwFields;

        explicit Slot(const RowMeta& meta, int identVal):
        _meta(meta),
        _ident(identVal){
            for (const auto& field: meta.getAllFields()){
                hwFields.push_back(&mOprReg(field._fieldName + "_ident_" + std::to_string(_ident), field._fieldSize));
            }
        }

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
                repFields.push_back(hwFields[i]);
            }
            SlotOpr slicedOpr(_meta(startIdx, stopIdx), repFields);
            return slicedOpr;

        }

        [[nodiscard]]
        SlotOpr getSlotOpr() const{
            return operator()(0, getAmtField());
        }

        int getAmtField()const{return _meta.getSize();}

        RowMeta getMeta()const{return _meta;}


        void assignCore(const SlotOpr& rhsSlotOpr, int startIdx, int stopIdx, Operable& condition){

            int size = stopIdx - startIdx;
            mfAssert(size == rhsSlotOpr.getAmtField(), "size is not equal");
            mfAssert(startIdx >= 0 && stopIdx <= hwFields.size(), "statIdx error");

            for(int i = 0; i < (stopIdx-startIdx); i++){
                FieldMeta lhsField = _meta.getField(startIdx+i);
                FieldMeta rhsField = rhsSlotOpr.getMeta().getField(i);
                mfAssert(lhsField.checkEqualField(rhsField),
                    "assign fieldNot equal fieldName lhs " + lhsField._fieldName + " rhs " + rhsField._fieldName);

                hwFields[startIdx + i]->addUpdateMeta(new UpdateEvent({
                    &condition,
                    nullptr,
                    rhsSlotOpr._repFields[i],
                    hwFields[startIdx + i]->getOperableSlice(),
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

        Slot& assign(const Slot& rhsSlot,
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

        Reg& get(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            return get(idx);
        }

        Reg& get(int idx){
            mfAssert(idx >= 0 && idx < hwFields.size(), "cannot find fieldIdx " + std::to_string(idx));
            return *hwFields[idx];
        }

    };



}

#endif //LIB_HW_SLOT_SLOT_H
