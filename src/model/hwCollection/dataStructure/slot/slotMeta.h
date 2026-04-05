//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H
#include "cassert"
#include "string"
#include "vector"
#include "algorithm"


#include "model/debugger/modelDebugger.h"

namespace kathryn{

    struct FieldMeta{
        std::string _name;
        int         _size;

        bool checkIntegrity() const{ return _size > 0;}

        bool operator == (const FieldMeta& rhs) const{
            return _name == rhs._name && _size == rhs._size;
        }

        bool operator != (const FieldMeta& rhs) const{
            return !(*this == rhs);
        }
    };

    struct SlotMeta{

        std::vector<FieldMeta>   _fieldMetas;

        /** the input start from LEAST significant bit to MOST sig
         * |valid|src|src1|
         *
         */
        SlotMeta()= default;

        SlotMeta(const std::vector<std::string>& fieldNames, const std::vector<int>& fieldSizes){
            /** we have to convert it to start from 0 to N */
            mfAssert(!fieldSizes.empty(), "field size cannot be empty");
            mfAssert(!fieldNames.empty(), "field name cannot be empty");
            mfAssert(fieldSizes.size() == fieldNames.size(), "field size and field name size not match");
            /** check fieldSize*/
            for (const int fieldSize: fieldSizes){
                mfAssert(fieldSize > 0, "field size cannot be negative or zero");
            }
            /** add to fieldMeta*/
            for (int idx = 0; idx < staticCast<int>(fieldSizes.size()); idx++){
                _fieldMetas.pushBack({fieldNames[idx], fieldSizes[idx]});
            }
        }

        SlotMeta(const std::vector<FieldMeta>& fieldMetas):
        _fieldMetas(fieldMetas){
            for (auto fieldMeta: _fieldMetas){
                mfAssert(fieldMeta.checkIntegrity(), "field meta " + fieldMeta._name + "is not valid");
            }
        }

        /** it will generate multiple start from {fieldPrefixName}_{startSuffixIdx}*/
        SlotMeta(std::string fieldPrefixName, int fieldwidth, int numField, int startSuffixIdx){

            for (int idx = 0; idx < numField; idx++){
                _fieldMetas.pushBack({fieldPrefixName + "_" + std::toString(idx + startSuffixIdx), fieldwidth});
                mfAssert(_fieldMetas.back().checkIntegrity(), "field meta " + _fieldMetas.back()._name + "is not valid");
            }

        }

        bool isValidIdx(int idx) const{
            return idx >= 0 && idx < staticCast<int>(_fieldMetas.size());
        }

        bool isThereField(const std::string& fieldName) const{
            return isValidIdx(getIdxPtr(fieldName));
        }

        bool isValidRange(int start, int end) const{
            return ( (start >= 0    ) && (start <  staticCast<int>(_fieldMetas.size())) )    &&
                   ( (end   >  start) && (end   <= staticCast<int>(_fieldMetas.size())) )   ;
        }

        bool isSufficientIdx(int amt) const{
            return (1 << amt) >= _fieldMetas.size();
        }

        int getNumFieldPtr() const{
            return _fieldMetas.size();
        }

        int getMaxBitWidth() const{
            int result = _fieldMetas[0]._size;
            for (const FieldMeta& fieldMeta: _fieldMetas){
                result = std::max(result, fieldMeta._size);
            }
            return result;
        }

        int getIdxPtr(std::string fieldName) const{
            for (int idx = 0; idx < staticCast<int>(_fieldMetas.size()); idx++){
                if (_fieldMetas[idx]._name == fieldName){
                    return idx;
                }
            }
            return -1;
        }

        std::vector<int> getIdxPtrs(std::vector<std::string> fieldNames) const{
            std::vector<int> result;
            for (auto fieldName: fieldNames){
                int idx = getIdxPtr(fieldName);
                result.pushBack(idx);
            }
            return result;
        }

        /** field operator */

        FieldMeta getField(int idx) const{
            mfAssert(isValidIdx(idx), "index" + std::toString(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta& operator()(int idx){
            mfAssert(isValidIdx(idx), "index" + std::toString(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta getCopyField(int idx) const{
            mfAssert(isValidIdx(idx), "index" + std::toString(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta& operator()(std::string fieldName){
            int fieldId = getIdxPtr(fieldName);
            mfAssert(isValidIdx(fieldId), "field name " + fieldName + " not found");
            return _fieldMetas[fieldId];
        }

        SlotMeta operator() (int start, int end){
            mfAssert(isValidIdx(start), "start index" + std::toString(start) + " out of range");
            int newAmt = end - start;
            mfAssert(newAmt > 0 && newAmt <= _fieldMetas.size(), "end index must be larger than start index");

            std::vector<FieldMeta> newFieldMetas;
            for (int idx = start; idx < end; idx++){
                newFieldMetas.pushBack(_fieldMetas[idx]);
            }
            return SlotMeta(newFieldMetas);

        }

        SlotMeta operator() (const std::vector<int>& fieldIdxs){
            std::vector<FieldMeta> newFieldMetas;
            for (auto idx: fieldIdxs){
                mfAssert(isValidIdx(idx), "index" + std::toString(idx) +  " out of range");
                newFieldMetas.pushBack(_fieldMetas[idx]);
            }
            return SlotMeta(newFieldMetas);
        }

        bool operator == (const SlotMeta& rhs){

            bool isEqual = true;
            if (getNumFieldPtr() != rhs.getNumFieldPtr()){
                return false;
            }

            for (int idx = 0; idx < getNumFieldPtr(); idx++){
                if (_fieldMetas[idx] != rhs._fieldMetas[idx]){
                    isEqual = false;
                    break;
                }
            }

            return isEqual;
        }


        SlotMeta operator + (const SlotMeta& rhs) const{
            SlotMeta result(*this);
            result._fieldMetas.insert(
                result._fieldMetas.end(),
                rhs._fieldMetas.begin(),
                rhs._fieldMetas.end());
            return result;
        }

        SlotMeta operator - (const std::vector<std::string>& deleteItem) const{

            std::vector<FieldMeta> newFieldMetas;
            for (const FieldMeta& fieldMeta: _fieldMetas){
                if (std::find(deleteItem.begin(), deleteItem.end(), fieldMeta._name) == deleteItem.end()){
                    newFieldMetas.pushBack(fieldMeta);
                }
            }
            return {newFieldMetas};

            
        }

        void addField(const FieldMeta& fieldMeta){
            assert(!isThereField(fieldMeta._name));
            _fieldMetas.pushBack(fieldMeta);
        }

        std::pair<std::vector<int>, std::vector<int>> matchByName(const SlotMeta& rhs){
            std::vector<int> srcMatchIdxs;
            std::vector<int> desMatchIdxs;

            for (int srcIdx = 0; srcIdx < rhs.getNumFieldPtr(); srcIdx++){
                std::string rhsFieldName = rhs.getField(srcIdx)._name;
                int desIdx = getIdxPtr(rhsFieldName);
                if (desIdx != -1){
                    srcMatchIdxs.pushBack(srcIdx);
                    desMatchIdxs.pushBack(desIdx);
                }
            }

            return {srcMatchIdxs, desMatchIdxs};
        }




    };


}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H