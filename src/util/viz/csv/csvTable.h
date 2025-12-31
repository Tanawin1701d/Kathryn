//
// Created by tanawin on 10/1/2025.
//

#ifndef src_util_viz_csv_CSVGEN_H
#define src_util_viz_csv_CSVGEN_H
#include <vector>
#include <algorithm>
#include <cassert>
#include "util/str/strUtil.h"




namespace kathryn{

    /**
     * current version we fix row and column size
     */

    constexpr int CSV_TABLE_NOT_FOUND_IDX = -1;
    struct CsvTable{

        //////
        /// |empty     |col_label1|col_label2|col_label3|
        /// |rowLabel1 |value     |value     |value     |
        /// |rowLabel2 |value     |value     |value     |
        /// |rowLabel3 |value     |value     |value     |
        /// |rowLabel4 |value     |value     |value     |

        const int AMTROW = -1;
        const int AMTCOL = -1;
        std::string                           _tableName;
        std::vector<std::vector<std::string>> _tableData;
        std::vector<std::string>              _columnLabels;
        std::vector<std::string>              _rowLabels;

        explicit CsvTable(int amtRow, int amtCol):
        AMTROW(amtRow), AMTCOL(amtCol){
            assert(amtRow > 0);
            assert(amtCol > 0);
            _tableData.resize(AMTROW);
            for (auto& row: _tableData){
                row.resize(amtCol);
            }
        }

        void setHeadNames(bool isRow, const std::vector<std::string>& indexName){
            int sz = isRow ? AMTROW : AMTCOL;
            std::vector<std::string>& targetVec = isRow ? _rowLabels : _columnLabels;

            assert(sz == indexName.size());
            targetVec = indexName;

        }

        void setTableName(const std::string& tableName){
            _tableName = tableName;
        }

        void setsHeadNameIterative(bool isRow, const std::string& prefix,
                                   int startValue = 0, int inc = 1){
            int sz = isRow ? AMTROW : AMTCOL;
            std::vector<std::string> result;
            result.reserve(sz);
            for(int i = 0; i < sz; i++){
                result.push_back(prefix + "_" + std::to_string(startValue + i*inc));
            }
            setHeadNames(isRow, result);

        }

        static int getIdFromVector(const std::vector<std::string>& targetVec, const std::string& name){
            auto resultIter = std::find(targetVec.begin(), targetVec.end(), name);
            if (resultIter == targetVec.end()){
                return CSV_TABLE_NOT_FOUND_IDX;
            }
            return static_cast<int>(resultIter - targetVec.begin());
        }

        void setData(int rowIdx, int colIdx, const std::string& value){
            assert(rowIdx < AMTROW && rowIdx >= 0);
            assert(colIdx < AMTCOL && colIdx >= 0);
            _tableData[rowIdx][colIdx] = value;
        }

        void setData(const std::string& rowName, const std::string& colName, const std::string& value){
            setData(getIdFromVector(_rowLabels, rowName),
                    getIdFromVector(_columnLabels, colName),
                    value
            );
        }

        std::string& getData(int rowIdx, int colIdx){
            assert(rowIdx < AMTROW && rowIdx >= 0);
            assert(colIdx < AMTCOL && colIdx >= 0);
            return _tableData[rowIdx][colIdx];
        }

        std::string& getData(const std::string& rowName, const std::string& colName){
            return getData(getIdFromVector(_rowLabels, rowName),
                           getIdFromVector(_columnLabels, colName));
        }

        [[nodiscard]] std::string genStr() const{
            std::string result = _tableName + "\n";

            /////// put column
            result += "," + joinStr(_columnLabels, ',') + "\n";

            for (int rowIdx = 0; rowIdx < _rowLabels.size(); rowIdx++){
                std::vector<std::string> actualRow = _tableData[rowIdx];
                actualRow.insert(actualRow.begin(), _rowLabels[rowIdx]);
                result += joinStr(actualRow, ',') + "\n";
            }
            return result;
        }

    };

}


#endif //src_util_viz_csv_CSVGEN_H
