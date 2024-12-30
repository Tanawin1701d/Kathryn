//
// Created by tanawin on 30_12_2024.
//

#ifndef src_carolyne_arch_base_march_abstract_FIELDS_H
#define src_carolyne_arch_base_march_abstract_FIELDS_H

#include<vector>
#include<map>
#include<string>
#include "model/hwComponent/abstract/Slice.h"
#include "carolyne/util/checker/checker.h"


namespace kathryn{
    namespace carolyne{
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

        struct RowMeta{
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

        };

    }
}

#endif //src_carolyne_arch_base_march_abstract_FIELDS_H
