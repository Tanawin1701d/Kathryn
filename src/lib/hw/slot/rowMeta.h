//
// Created by tanawin on 16/1/2025.
//

#ifndef LIB_HW_SLOT_ROWMETA_H
#define LIB_HW_SLOT_ROWMETA_H

#include <string>
#include <vector>
#include "csvVizable.h"
#include "model/debugger/modelDebugger.h"
#include "model/hwComponent/abstract/Slice.h"


namespace kathryn{
    constexpr int MAX_FIELD_SIZE = 64;
    struct FieldMeta{
        std::string _fieldName = "unname";
        int         _fieldSize = -1;
        FieldMeta(const std::string& fieldName, int fieldSize):
                _fieldName(fieldName), _fieldSize(fieldSize){
            mfAssert(fieldSize > 0 && fieldSize <= MAX_FIELD_SIZE, "field "  + fieldName + " size error ");
        }

        bool checkEqualField(const FieldMeta& rhs){
            return (_fieldName == rhs._fieldName) &&
                   (_fieldSize == rhs._fieldSize);
        }
    };

    struct RowMeta : VizCsvGenTable{
        std::vector<FieldMeta> _fields;
        int totalSize = 0;

        RowMeta(const std::vector<std::string>& nms,
                const std::vector<int>&         szs){
            mfAssert(nms.size() == szs.size(), "field and size not match");
            for(int i = 0; i < nms.size(); i++){addField(nms[i], szs[i]);}
        }

        void addField(const std::string& nm, int sz){
            _fields.emplace_back(nm, sz);
            totalSize += sz;
        }

        int getSize() const{return static_cast<int>(_fields.size());}

        FieldMeta getField(int idx) const{
            mfAssert(isThereIdx(idx), "cannot get field idx " + std::to_string(idx));
            return _fields[idx];
        }

        [[nodiscard]]
        FieldMeta getField(const std::string& fieldName)const{
            int idx = 0;
            for (const FieldMeta& fm: _fields){
                idx++;
                if (fm._fieldName == fieldName){
                    break;
                }
            }
            return getField(idx);
        }

        [[nodiscard]]
        int getFieldIdx(const std::string& fieldName)const{
            int idx = 0;
            for (const FieldMeta& fm: _fields){
                if (fm._fieldName == fieldName){return idx;}
            }
            return -1;
        }

        bool isThereIdx(int idx) const{
            return (idx >= 0) && (idx < _fields.size());
        }

        RowMeta operator() (int startIdx, int stopIdx)const{
            std::vector<std::string> newNms;
            std::vector<int>         newSzs;
            mfAssert(isThereIdx(startIdx) && isThereIdx(stopIdx-1),
                "cannot slice rowMeta "+
                std::to_string(startIdx) + " : " + std::to_string(stopIdx));
            for (int i = startIdx; i < stopIdx; i++){
                newNms.push_back(_fields[i]._fieldName);
                newSzs.push_back(_fields[i]._fieldSize);
            }
            RowMeta newRowMeta(newNms, newSzs);
            return newRowMeta;
        }



        [[nodiscard]]
        std::vector<FieldMeta> getAllFields() const{
            return _fields;
        }

        std::vector<std::string> getAllFieldNames(){
            std::vector<std::string> names;
            names.reserve(_fields.size());
            for (auto& field: _fields){
                names.push_back(field._fieldName);
            }
            return names;
        }

        std::vector<int> getAllFieldSizes(){
            std::vector<int> sizes;
            sizes.reserve(_fields.size());
            for (auto& field: _fields){
                sizes.push_back(field._fieldSize);
            }
            return sizes;
        }

        [[nodiscard]]
        Slice getFieldSlice(const std::string& fieldName)const{
            int preSize = 0;
            for (const FieldMeta& fm: _fields){
                if (fm._fieldName == fieldName){
                    return {preSize, preSize + fm._fieldSize};
                }
                preSize += fm._fieldSize;
            }
            mfAssert(false, "can't find field By name: " + fieldName);
            return {};
        }

        [[nodiscard]]
        Slice getSliceAll() const{
            return {0, totalSize};
        }

        void reverse(){
            std::reverse(_fields.begin(), _fields.end());
        }

        RowMeta& operator += (const RowMeta& rhs){
            for (const FieldMeta& cpyField: rhs._fields){ _fields.push_back(cpyField);}
            return *this;
        }

        CsvTable genTable() override{

            constexpr char RN_BITWIDTH[] = "bitWidth";
            CsvTable table(1, static_cast<int>(_fields.size()));
            std::vector<std::string> colNames = getAllFieldNames();
            table.setHeadNames(true, {RN_BITWIDTH});
            table.setHeadNames(false, colNames);
            ////// set value
            for (FieldMeta& field: _fields){
                table.setData(RN_BITWIDTH,
                              field._fieldName,
                              std::to_string(field._fieldSize));
            }
            return table;

        }

        bool checkEqualRowMeta(const RowMeta& rhs, int startIdx, int stopIdx, int rhsStartIdx){
            mfAssert((startIdx < stopIdx) &&
                     (startIdx >= 0) &&
                     (stopIdx <= _fields.size()), "ill form of rowMeta checking");

            if (!(    isThereIdx(startIdx)    &&     isThereIdx(stopIdx) &&
                  rhs.isThereIdx(rhsStartIdx) && rhs.isThereIdx(rhsStartIdx + (stopIdx - startIdx)))
            ){return false;}

            for (int i = 0; i < (stopIdx-startIdx); i++){
                if (!_fields[startIdx + i].checkEqualField(rhs._fields[rhsStartIdx + i])){
                    return false;
                }
            }
            return true;
        }

        bool checkEqualRowMeta(const RowMeta& rhs,
                               const std::string& startFieldName,
                               const std::string& stopFieldName){

            int startIdx    = getFieldIdx    (startFieldName);
            int stopIdx     = getFieldIdx    (stopFieldName);
            int rhsStartIdx = rhs.getFieldIdx(startFieldName);
            if (stopIdx == -1){return false;}
            stopIdx++; /// we require excluded index

            return checkEqualRowMeta(rhs, startIdx, stopIdx, rhsStartIdx);
        }

    };
}

#endif //LIB_HW_SLOT_ROWMETA_H
