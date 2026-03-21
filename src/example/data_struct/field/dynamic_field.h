//
// Created by tanawin on 4/10/2024.
//

#ifndef src_example_dataStruct_field_DYNAMICFIELD_H
#define src_example_dataStruct_field_DYNAMICFIELD_H


namespace kathryn{

    struct DYNAMIC_FIELD{
        std::vector<std::string> _valueFieldNames{};
        std::vector<int>         _valueFieldSizes{};

        DYNAMIC_FIELD(const std::vector<std::string>& value_field_names,
                      const std::vector<int>& value_field_sizes):
        _valueFieldNames(value_field_names),
        _valueFieldSizes(value_field_sizes){
            assert(_valueFieldNames.size() == _valueFieldSizes.size());
            for (int sz: _valueFieldSizes){
                assert(sz > 0);
            }
        }

        int sum_field_size(){
            int acc = 0;
            for (int sz: _valueFieldSizes){
                assert(sz > 0);
                acc += sz;
            }
            return acc;
        }

        int amt_field(){
            return _valueFieldNames.size();
        }

        DYNAMIC_FIELD operator +(const DYNAMIC_FIELD& rhs){
            DYNAMIC_FIELD new_d = *this;
            for (int i = 0; i < rhs._valueFieldNames.size(); i++){
                new_d._valueFieldNames.push_back(rhs._valueFieldNames[i]);
                new_d._valueFieldSizes.push_back(rhs._valueFieldSizes[i]);
            }
            return new_d;
        }

        DYNAMIC_FIELD& reverse(){
            std::reverse(_valueFieldNames.begin(), _valueFieldNames.end());
            std::reverse(_valueFieldSizes.begin(), _valueFieldSizes.end());
            return *this;
        }

        int find_idx(const std::string& name){
            auto find_iter = std::find(_valueFieldNames.begin(), _valueFieldNames.end(), name);
            assert(find_iter != _valueFieldNames.end());
            size_t find_idx = std::distance(_valueFieldNames.begin(), find_iter);
            return find_idx;
        }

        int find_start_bit(int idx){
            assert(idx < _valueFieldNames.size());
            int result = 0;
            for (int cur_idx = 0; cur_idx < idx; cur_idx++){
                result += _valueFieldSizes[cur_idx];
            }
            return result;
        }

        int get_size(int idx){
            assert(idx < _valueFieldNames.size());
            return _valueFieldSizes[idx];
        }

        std::pair<std::string, int> get_key_size(int idx){
            assert(idx < _valueFieldNames.size());
            return {_valueFieldNames[idx], _valueFieldSizes[idx]};
        }

        std::vector<std::pair<std::string, int>> get_all_key_size(){
            std::vector<std::pair<std::string, int>> result;
            for (int i = 0; i < _valueFieldNames.size(); i++){
                result.push_back(get_key_size(i));
            }
            return result;
        }

    };




}

#endif //src_example_dataStruct_field_DYNAMICFIELD_H
