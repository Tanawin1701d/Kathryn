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
        RowMeta _meta;
        int     _ident;

        std::vector<Reg*> hwFields;
        mPmVal(identParam, 0);

        explicit Slot(const RowMeta& meta, int identVal):
        _meta(meta),
        _ident(identVal){

            for (const auto& field: meta.getAllFields()){
                hwFields.push_back(&makeOprReg(field._fieldName, field._fieldSize));
            }
        }

        [[nodiscard]]
        bool isContainIdx(int startIdx, int stopIdx)const{
            mfAssert((startIdx >= 0) && (startIdx < stopIdx), "isContainIdx have invalid input");
            return stopIdx <= hwFields.size();
        }

        SlotOpr operator() (int startIdx, int stopIdx)const{

            mfAssert(_meta.isThereIdx(startIdx) && _meta.isThereIdx(stopIdx-1),
                "slice slot opr index out of range");

            std::vector<Operable*> repFields;
            for (int i = startIdx; i < stopIdx; i++){
                repFields.push_back(hwFields[i]);
            }
            SlotOpr slicedOpr(_meta(startIdx, stopIdx), repFields);
            return slicedOpr;

        }

        int getAmtField()const{return _meta.getSize();}

        void assignCore(const SlotOpr& rhsSlotOpr, int startIdx, int stopIdx, bool isNextCycle, Operable& condition){

            int size = stopIdx - startIdx;
            mfAssert(size == rhsSlotOpr.getAmtField(), "size is not equal");
            mfAssert(startIdx >= 0 && stopIdx <= hwFields.size(), "statIdx error");

            zif (condition){
                for(int i = 0; i < (stopIdx-startIdx); i++){
                    FieldMeta lhsField = _meta.getField(startIdx+i);
                    FieldMeta rhsField = rhsSlotOpr._meta.getField(i);
                    mfAssert(lhsField.checkEqualField(rhsField), "assign fieldNot equal fieldName lhs " + lhsField._fieldName + " rhs " + rhsField._fieldName);
                    if (isNextCycle){(*hwFields[startIdx + i]) <<= (*rhsSlotOpr._repFields[i]);}
                    else            {(*hwFields[startIdx + i])   = (*rhsSlotOpr._repFields[i]);}
                }
            }

        }

        void assignCore(const Slot& rhsSlot,
                         int startIdx, int stopIdx, int rhsStartIdx,
                         bool isNextCycle, Operable& condition){
            assignCore(rhsSlot(rhsStartIdx, getAmtField()), startIdx, stopIdx, isNextCycle, condition);
        }

        Slot& assign(const Slot& rhsSlot,
                     const std::string& startFieldName,
                     const std::string& endFieldName,
                     bool  isNextCycle, Operable& condition){ ///[startFieldName, endFieldName]
            ///// get the index and assign it
            int startIdx = _meta.getFieldIdx(startFieldName);
            int stopIdx  = _meta.getFieldIdx(endFieldName);
            stopIdx++;
            int rhsStartIdx = rhsSlot._meta.getFieldIdx(startFieldName);
            assignCore(rhsSlot, startIdx, stopIdx, rhsStartIdx, isNextCycle, condition);

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
