//
// Created by tanawin on 30_12_2024.
//

#ifndef src_carolyne_arch_base_march_abstract_FIELDS_H
#define src_carolyne_arch_base_march_abstract_FIELDS_H

#include <algorithm>
#include<vector>
#include<map>
#include<string>
#include "model/hwComponent/abstract/Slice.h"
#include "carolyne/util/checker/checker.h"
#include "carolyne/util/viz/csvVizable.h"


namespace kathryn::carolyne{
        /**
         * RowMeta is abstract structure representing
         * the data slot and its meaning
         */
        constexpr int MAX_FIELD_SIZE = 64;
        struct FieldMeta{
            std::string _fieldName = "unname";
            int         _fieldSize = -1;
            FieldMeta(const std::string& fieldName, int fieldSize):
             _fieldName(fieldName), _fieldSize(fieldSize){
                crlAss(fieldSize > 0 && fieldSize <= MAX_FIELD_SIZE, "field "  + fieldName + " size error ");
            }
        };

        struct RowMeta : VizCsvGenTable{
            std::vector<FieldMeta> _fields;
            int totalSize = 0;

            void addField(const std::string& nm, int sz){
                _fields.emplace_back(nm, sz);
                totalSize += sz;
            }

            [[nodiscard]]
            FieldMeta getField(const std::string& fieldName)const{
                for (const FieldMeta& fm: _fields){
                    if (fm._fieldName == fieldName){
                        return fm;
                    }
                }
                crlAss(false, "can't find field By name: " + fieldName);
                return {"error", 20};
            }

            std::vector<std::string> getAllFieldNames(){
                std::vector<std::string> names;
                names.reserve(_fields.size());
                for (auto& field: _fields){
                    names.push_back(field._fieldName);
                }
                return names;
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
                crlAss(false, "can't find field By name: " + fieldName);
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

        };

        enum CRL_GEN_MODE{
            CGM_FETCH,
            CGM_DECODE,
            CGM_ALLOC,
            CGM_RSV,
            CGM_EXEC,
            CGM_ROB
        };

        struct GenRowMetaable{
            virtual ~GenRowMetaable() = default;
            virtual RowMeta genRowMeta(CRL_GEN_MODE genMode, int subMode) = 0;
            virtual RowMeta genRowMeta(const std::string& genMode) = 0;
        };

    }


#endif //src_carolyne_arch_base_march_abstract_FIELDS_H
