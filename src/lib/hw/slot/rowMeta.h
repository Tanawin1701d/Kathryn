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


namespace kathryn {
    constexpr int MAX_FIELD_SIZE = 64;

    struct FieldMeta {
        std::string _fieldName = "unname";
        int _fieldSize = -1;

        FieldMeta(const std::string &fieldName, int fieldSize) :
                _fieldName(fieldName),
                _fieldSize(fieldSize) {
            mfAssert(fieldSize > 0 && fieldSize <= MAX_FIELD_SIZE, "field " + fieldName + " size error ");
        }

        bool operator==(const FieldMeta &rhs) const {
            return (_fieldName == rhs._fieldName) && (_fieldSize == rhs._fieldSize);
        }
    };

    struct RowMeta : VizCsvGenTable {
        std::vector<FieldMeta> _fields; ///// the order of the field is from LSB to MSB
        int _totalSize = 0; ////// cache the total bit size

        RowMeta() = default;

        RowMeta(const std::vector<std::string> &nms,
                const std::vector<int> &szs) {
            mfAssert(nms.size() == szs.size(), "field and size not match");
            for (int i = 0; i < nms.size(); i++) { addField(nms[i], szs[i]); }
        }

        RowMeta(const std::vector<FieldMeta> &fields) {
            for (const FieldMeta &fieldMeta: fields) { addField(fieldMeta); }
        }

        void addField(const std::string &nm, int sz) {
            _fields.emplace_back(nm, sz);
            _totalSize += sz;
        }

        void addField(const FieldMeta &fieldMeta) {
            _fields.emplace_back(fieldMeta);
            _totalSize += fieldMeta._fieldSize;
        }

        int getAmtField() const { return static_cast<int>(_fields.size()); }

        [[nodiscard]]
        FieldMeta getField(int idx) const {
            mfAssert(isThereIdx(idx), "cannot get field idx " + std::to_string(idx));
            return _fields[idx];
        }

        [[nodiscard]]
        FieldMeta getField(const std::string &fieldName) const {
            int targetIdx = getFieldIdx(fieldName);
            return getField(targetIdx);
        }

        [[nodiscard]]
        int getFieldIdx(const std::string &fieldName) const {
            int idx = 0;
            for (const FieldMeta &fm: _fields) {
                if (fm._fieldName == fieldName) { return idx; }
                idx++;
            }
            return -1;
        }

        [[nodiscard]]
        bool isThereIdx(int idx) const {
            return (idx >= 0) && (idx < _fields.size());
        }

        RowMeta operator()(int startIdx, int stopIdx) const {
            mfAssert(isThereIdx(startIdx) && isThereIdx(stopIdx - 1),
                     "cannot slice rowMeta " +
                     std::to_string(startIdx) + " : " + std::to_string(stopIdx));
            RowMeta newRowMeta;
            for (int i = startIdx; i < stopIdx; i++) { newRowMeta.addField(getField(i)); }
            return newRowMeta;
        }

        [[nodiscard]]
        std::vector<FieldMeta> getAllFields() const {
            return _fields;
        }

        std::vector<std::string> getAllFieldNames() const {
            std::vector<std::string> names;
            names.reserve(_fields.size());
            for (auto &field: _fields) {
                names.push_back(field._fieldName);
            }
            return names;
        }

        std::vector<int> getAllFieldSizes() const {
            std::vector<int> sizes;
            sizes.reserve(_fields.size());
            for (auto &field: _fields) {
                sizes.push_back(field._fieldSize);
            }
            return sizes;
        }

        RowMeta &operator+=(const RowMeta &rhs) {
            for (const FieldMeta &cpyField: rhs._fields) { _fields.push_back(cpyField); }
            return *this;
        }

        RowMeta operator+(const RowMeta& rhs) const{
            std::vector<FieldMeta> newFiledMetas = _fields;
            for (const FieldMeta &cpyField: rhs._fields){ newFiledMetas.push_back(cpyField);}
            return RowMeta(newFiledMetas);
        }

        CsvTable genTable() override {

            constexpr char RN_BITWIDTH[] = "bitWidth";
            CsvTable table(1, getAmtField());
            std::vector<std::string> colNames = getAllFieldNames();
            table.setHeadNames(true, {RN_BITWIDTH});
            table.setHeadNames(false, colNames);
            ////// set value
            for (FieldMeta &field: _fields) {
                table.setData(RN_BITWIDTH,
                              field._fieldName,
                              std::to_string(field._fieldSize));
            }
            return table;
        }

    };
}

#endif //LIB_HW_SLOT_ROWMETA_H
