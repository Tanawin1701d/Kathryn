//
// Created by tanawin on 16/1/2025.
//

#ifndef LIB_HW_SLOT_SLOT_H
#define LIB_HW_SLOT_SLOT_H

#include "model/controller/controller.h"
#include "rowMeta.h"

namespace kathryn{



    class Slot: public Module{
    public:
        RowMeta _meta;
        int     _ident;

        std::vector<Reg*> hwFields;
        mPmVal(identParam, 0);

        Slot(const RowMeta& meta, int identVal):
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

        Slot& assignCore(const Slot& rhsSlot,
                         int startIdx,
                         int stopIdx,
                         int rhsStartIdx,
                         bool isNextCycle){

            mfAssert(isContainIdx(startIdx, stopIdx), "assignCore:: error assign idx missmatch");
            mfAssert(isContainIdx(rhsStartIdx, rhsStartIdx + (stopIdx-startIdx)),
                     "assignCore:: error assign idx missmatch");

            for (int i = 0; i < (stopIdx-startIdx); i++){
                if (isNextCycle){(*hwFields[startIdx + i]) <<= (*rhsSlot.hwFields[rhsStartIdx + i]);}
                else            {(*hwFields[startIdx + i])   = (*rhsSlot.hwFields[rhsStartIdx + i]);}
            }

            return *this;
        }

        Slot& assign(const Slot& rhsSlot,
                     const std::string& startFieldName,
                     const std::string& endFieldName,
                     bool  isNextCycle){ ///[startFieldName, endFieldName]
                 /////// isEndCycle  = <<= else is =

            /////// check the equality first
            mfAssert(_meta.checkEqualRowMeta(rhsSlot._meta, startFieldName, endFieldName), "assign slot error");

            ///// get the index and assign it
            int startIdx = _meta.getFieldIdx(startFieldName);
            int stopIdx  = _meta.getFieldIdx(endFieldName);
            stopIdx++;
            int rhsStartIdx = rhsSlot._meta.getFieldIdx(startFieldName);
            assignCore(rhsSlot, startIdx, stopIdx, rhsStartIdx, isNextCycle);

            return *this;
        }

        Slot& assign(const Slot& rhsSlot, const std::string& startFieldName, bool isNextCycle){
            int startIdx    = _meta.getFieldIdx(startFieldName);
            int rhsStartIdx = rhsSlot._meta.getFieldIdx(startFieldName);
            assignCore(rhsSlot, startIdx, startIdx + 1, rhsStartIdx, isNextCycle);
            return *this;
        }

        Reg& get(const std::string& fieldName){
            int idx = _meta.getFieldIdx(fieldName);
            mfAssert(idx >= 0, "cannot find fieldName " + fieldName);
            return *hwFields[idx];
        }

        


    };



}

#endif //LIB_HW_SLOT_SLOT_H
