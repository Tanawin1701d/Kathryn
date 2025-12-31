//
// Created by tanawin on 4/10/2024.
//

#ifndef src_example_dataStruct_field_DYNAMICFIELD_H
#define src_example_dataStruct_field_DYNAMICFIELD_H


namespace kathryn{

    struct DYNAMIC_FIELD{
        std::vector<std::string> _valueFieldNames{};
        std::vector<int>         _valueFieldSizes{};

        DYNAMIC_FIELD(const std::vector<std::string>& valueFieldNames,
                      const std::vector<int>& valueFieldSizes):
        _valueFieldNames(valueFieldNames),
        _valueFieldSizes(valueFieldSizes){
            assert(_valueFieldNames.size() == _valueFieldSizes.size());
            for (int sz: _valueFieldSizes){
                assert(sz > 0);
            }
        }

        int sumFieldSize(){
            int acc = 0;
            for (int sz: _valueFieldSizes){
                assert(sz > 0);
                acc += sz;
            }
            return acc;
        }

        int amtField(){
            return _valueFieldNames.size();
        }

        DYNAMIC_FIELD operator +(const DYNAMIC_FIELD& rhs){
            DYNAMIC_FIELD newD = *this;
            for (int i = 0; i < rhs._valueFieldNames.size(); i++){
                newD._valueFieldNames.push_back(rhs._valueFieldNames[i]);
                newD._valueFieldSizes.push_back(rhs._valueFieldSizes[i]);
            }
            return newD;
        }

        DYNAMIC_FIELD& reverse(){
            std::reverse(_valueFieldNames.begin(), _valueFieldNames.end());
            std::reverse(_valueFieldSizes.begin(), _valueFieldSizes.end());
            return *this;
        }

        int findIdx(const std::string& name){
            auto findIter = std::find(_valueFieldNames.begin(), _valueFieldNames.end(), name);
            assert(findIter != _valueFieldNames.end());
            size_t findIdx = std::distance(_valueFieldNames.begin(), findIter);
            return findIdx;
        }

        int findStartBit(int idx){
            assert(idx < _valueFieldNames.size());
            int result = 0;
            for (int curIdx = 0; curIdx < idx; curIdx++){
                result += _valueFieldSizes[curIdx];
            }
            return result;
        }

        int getSize(int idx){
            assert(idx < _valueFieldNames.size());
            return _valueFieldSizes[idx];
        }

        std::pair<std::string, int> getKeySize(int idx){
            assert(idx < _valueFieldNames.size());
            return {_valueFieldNames[idx], _valueFieldSizes[idx]};
        }

        std::vector<std::pair<std::string, int>> getAllKeySize(){
            std::vector<std::pair<std::string, int>> result;
            for (int i = 0; i < _valueFieldNames.size(); i++){
                result.push_back(getKeySize(i));
            }
            return result;
        }

    };




}

#endif //src_example_dataStruct_field_DYNAMICFIELD_H
