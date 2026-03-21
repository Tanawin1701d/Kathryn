//
// Created by tanawin on 10/1/2025.
//

#ifndef src_util_viz_csv_CSVGEN_H
#define src_util_viz_csv_CSVGEN_H
#include "vector"
#include "algorithm"
#include "cassert"
#include "util/str/str_util.h"




namespace kathryn{

    /**
     * current version we fix row and column size
     */

    constexpr int CSV_TABLE_NOT_FOUND_IDX = -1;
    struct CsvTable{

        //////
        /// |empty     |col_label1|col_label2|col_label3|
        /// |row_label1 |value     |value     |value     |
        /// |row_label2 |value     |value     |value     |
        /// |row_label3 |value     |value     |value     |
        /// |row_label4 |value     |value     |value     |

        const int AMTROW = -1;
        const int AMTCOL = -1;
        std::string                           _tableName;
        std::vector<std::vector<std::string>> _tableData;
        std::vector<std::string>              _columnLabels;
        std::vector<std::string>              _rowLabels;

        explicit CsvTable(int amt_row, int amt_col):
        AMTROW(amt_row), AMTCOL(amt_col){
            assert(amt_row > 0);
            assert(amt_col > 0);
            _tableData.resize(AMTROW);
            for (auto& row: _tableData){
                row.resize(amt_col);
            }
        }

        void set_head_names(bool is_row, const std::vector<std::string>& index_name){
            int sz = is_row ? AMTROW : AMTCOL;
            std::vector<std::string>& target_vec = is_row ? _rowLabels : _columnLabels;

            assert(sz == index_name.size());
            target_vec = index_name;

        }

        void set_table_name(const std::string& table_name){
            _tableName = table_name;
        }

        void sets_head_name_iterative(bool is_row, const std::string& prefix,
                                   int start_value = 0, int inc = 1){
            int sz = is_row ? AMTROW : AMTCOL;
            std::vector<std::string> result;
            result.reserve(sz);
            for(int i = 0; i < sz; i++){
                result.push_back(prefix + "_" + std::to_string(start_value + i*inc));
            }
            set_head_names(is_row, result);

        }

        static int get_id_from_vector(const std::vector<std::string>& target_vec, const std::string& name){
            auto result_iter = std::find(target_vec.begin(), target_vec.end(), name);
            if (result_iter == target_vec.end()){
                return CSV_TABLE_NOT_FOUND_IDX;
            }
            return static_cast<int>(result_iter - target_vec.begin());
        }

        void set_data(int row_idx, int col_idx, const std::string& value){
            assert(row_idx < AMTROW && row_idx >= 0);
            assert(col_idx < AMTCOL && col_idx >= 0);
            _tableData[row_idx][col_idx] = value;
        }

        void set_data(const std::string& row_name, const std::string& col_name, const std::string& value){
            set_data(get_id_from_vector(_rowLabels, row_name),
                    get_id_from_vector(_columnLabels, col_name),
                    value
            );
        }

        std::string& get_data(int row_idx, int col_idx){
            assert(row_idx < AMTROW && row_idx >= 0);
            assert(col_idx < AMTCOL && col_idx >= 0);
            return _tableData[row_idx][col_idx];
        }

        std::string& get_data(const std::string& row_name, const std::string& col_name){
            return get_data(get_id_from_vector(_rowLabels, row_name),
                           get_id_from_vector(_columnLabels, col_name));
        }

        [[nodiscard]] std::string gen_str() const{
            std::string result = _tableName + "\n";

            /////// put column
            result += "," + join_str(_columnLabels, ',') + "\n";

            for (int row_idx = 0; row_idx < _rowLabels.size(); row_idx++){
                std::vector<std::string> actual_row = _tableData[row_idx];
                actual_row.insert(actual_row.begin(), _rowLabels[row_idx]);
                result += join_str(actual_row, ',') + "\n";
            }
            return result;
        }

    };

}


#endif //src_util_viz_csv_CSVGEN_H
