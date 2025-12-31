//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H
#include <cassert>
#include <string>
#include <vector>

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
            for (int idx = 0; idx < static_cast<int>(fieldSizes.size()); idx++){
                _fieldMetas.push_back({fieldNames[idx], fieldSizes[idx]});
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
                _fieldMetas.push_back({fieldPrefixName + "_" + std::to_string(idx + startSuffixIdx), fieldwidth});
                mfAssert(_fieldMetas.back().checkIntegrity(), "field meta " + _fieldMetas.back()._name + "is not valid");
            }

        }

        bool isValidIdx(int idx) const{
            return idx >= 0 && idx < static_cast<int>(_fieldMetas.size());
        }

        bool isThereField(const std::string& fieldName) const{
            return isValidIdx(getIdx(fieldName));
        }

        bool isValidRange(int start, int end) const{
            return ( (start >= 0    ) && (start <  static_cast<int>(_fieldMetas.size())) )    &&
                   ( (end   >  start) && (end   <= static_cast<int>(_fieldMetas.size())) )   ;
        }

        bool isSufficientIdx(int amt) const{
            return (1 << amt) >= _fieldMetas.size();
        }

        int getNumField() const{
            return _fieldMetas.size();
        }

        int getMaxBitWidth() const{
            int result = _fieldMetas[0]._size;
            for (const FieldMeta& fieldMeta: _fieldMetas){
                result = std::max(result, fieldMeta._size);
            }
            return result;
        }

        int getIdx(std::string fieldName) const{
            for (int idx = 0; idx < static_cast<int>(_fieldMetas.size()); idx++){
                if (_fieldMetas[idx]._name == fieldName){
                    return idx;
                }
            }
            return -1;
        }

        std::vector<int> getIdxs(std::vector<std::string> fieldNames) const{
            std::vector<int> result;
            for (auto fieldName: fieldNames){
                int idx = getIdx(fieldName);
                result.push_back(idx);
            }
            return result;
        }

        /** field operator */

        FieldMeta getField(int idx) const{
            mfAssert(isValidIdx(idx), "index" + std::to_string(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta& operator()(int idx){
            mfAssert(isValidIdx(idx), "index" + std::to_string(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta getCopyField(int idx) const{
            mfAssert(isValidIdx(idx), "index" + std::to_string(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta& operator()(std::string fieldName){
            int fieldId = getIdx(fieldName);
            mfAssert(isValidIdx(fieldId), "field name " + fieldName + " not found");
            return _fieldMetas[fieldId];
        }

        SlotMeta operator() (int start, int end){
            mfAssert(isValidIdx(start), "start index" + std::to_string(start) + " out of range");
            int newAmt = end - start;
            mfAssert(newAmt > 0 && newAmt <= _fieldMetas.size(), "end index must be larger than start index");

            std::vector<FieldMeta> newFieldMetas;
            for (int idx = start; idx < end; idx++){
                newFieldMetas.push_back(_fieldMetas[idx]);
            }
            return SlotMeta(newFieldMetas);

        }

        SlotMeta operator() (const std::vector<int>& fieldIdxs){
            std::vector<FieldMeta> newFieldMetas;
            for (auto idx: fieldIdxs){
                mfAssert(isValidIdx(idx), "index" + std::to_string(idx) +  " out of range");
                newFieldMetas.push_back(_fieldMetas[idx]);
            }
            return SlotMeta(newFieldMetas);
        }

        bool operator == (const SlotMeta& rhs){

            bool isEqual = true;
            if (getNumField() != rhs.getNumField()){
                return false;
            }

            for (int idx = 0; idx < getNumField(); idx++){
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

        void addField(const FieldMeta& fieldMeta){
            assert(!isThereField(fieldMeta._name));
            _fieldMetas.push_back(fieldMeta);
        }

        std::pair<std::vector<int>, std::vector<int>> matchByName(const SlotMeta& rhs){
            std::vector<int> srcMatchIdxs;
            std::vector<int> desMatchIdxs;

            for (int srcIdx = 0; srcIdx < rhs.getNumField(); srcIdx++){
                std::string rhsFieldName = rhs.getField(srcIdx)._name;
                int desIdx = getIdx(rhsFieldName);
                if (desIdx != -1){
                    srcMatchIdxs.push_back(srcIdx);
                    desMatchIdxs.push_back(desIdx);
                }
            }

            return {srcMatchIdxs, desMatchIdxs};
        }




    };


}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H