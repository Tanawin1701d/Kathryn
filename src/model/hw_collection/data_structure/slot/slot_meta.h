//
// Created by tanawin on 14/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H
#include "cassert"
#include "string"
#include "vector"
#include "algorithm"


#include "model/debugger/model_debugger.h"

namespace kathryn{

    struct FieldMeta{
        std::string _name;
        int         _size;

        bool check_integrity() const{ return _size > 0;}

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

        SlotMeta(const std::vector<std::string>& field_names, const std::vector<int>& field_sizes){
            /** we have to convert it to start from 0 to N */
            mf_assert(!field_sizes.empty(), "field size cannot be empty");
            mf_assert(!field_names.empty(), "field name cannot be empty");
            mf_assert(field_sizes.size() == field_names.size(), "field size and field name size not match");
            /** check field_size*/
            for (const int field_size: field_sizes){
                mf_assert(field_size > 0, "field size cannot be negative or zero");
            }
            /** add to field_meta*/
            for (int idx = 0; idx < static_cast<int>(field_sizes.size()); idx++){
                _fieldMetas.push_back({field_names[idx], field_sizes[idx]});
            }
        }

        SlotMeta(const std::vector<FieldMeta>& field_metas):
        _fieldMetas(field_metas){
            for (auto field_meta: _fieldMetas){
                mf_assert(field_meta.check_integrity(), "field meta " + field_meta._name + "is not valid");
            }
        }

        /** it will generate multiple start from {field_prefix_name}_{start_suffix_idx}*/
        SlotMeta(std::string field_prefix_name, int fieldwidth, int num_field, int start_suffix_idx){

            for (int idx = 0; idx < num_field; idx++){
                _fieldMetas.push_back({field_prefix_name + "_" + std::to_string(idx + start_suffix_idx), fieldwidth});
                mf_assert(_fieldMetas.back().check_integrity(), "field meta " + _fieldMetas.back()._name + "is not valid");
            }

        }

        bool is_valid_idx(int idx) const{
            return idx >= 0 && idx < static_cast<int>(_fieldMetas.size());
        }

        bool is_there_field(const std::string& field_name) const{
            return is_valid_idx(get_idx_ptr(field_name));
        }

        bool is_valid_range(int start, int end) const{
            return ( (start >= 0    ) && (start <  static_cast<int>(_fieldMetas.size())) )    &&
                   ( (end   >  start) && (end   <= static_cast<int>(_fieldMetas.size())) )   ;
        }

        bool is_sufficient_idx(int amt) const{
            return (1 << amt) >= _fieldMetas.size();
        }

        int get_num_field_ptr() const{
            return _fieldMetas.size();
        }

        int get_max_bit_width() const{
            int result = _fieldMetas[0]._size;
            for (const FieldMeta& field_meta: _fieldMetas){
                result = std::max(result, field_meta._size);
            }
            return result;
        }

        int get_idx_ptr(std::string field_name) const{
            for (int idx = 0; idx < static_cast<int>(_fieldMetas.size()); idx++){
                if (_fieldMetas[idx]._name == field_name){
                    return idx;
                }
            }
            return -1;
        }

        std::vector<int> get_idx_ptrs(std::vector<std::string> field_names) const{
            std::vector<int> result;
            for (auto field_name: field_names){
                int idx = get_idx_ptr(field_name);
                result.push_back(idx);
            }
            return result;
        }

        /** field operator */

        FieldMeta get_field(int idx) const{
            mf_assert(is_valid_idx(idx), "index" + std::to_string(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta& operator()(int idx){
            mf_assert(is_valid_idx(idx), "index" + std::to_string(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta get_copy_field(int idx) const{
            mf_assert(is_valid_idx(idx), "index" + std::to_string(idx) +  " out of range");
            return _fieldMetas[idx];
        }

        FieldMeta& operator()(std::string field_name){
            int field_id = get_idx_ptr(field_name);
            mf_assert(is_valid_idx(field_id), "field name " + field_name + " not found");
            return _fieldMetas[field_id];
        }

        SlotMeta operator() (int start, int end){
            mf_assert(is_valid_idx(start), "start index" + std::to_string(start) + " out of range");
            int new_amt = end - start;
            mf_assert(new_amt > 0 && new_amt <= _fieldMetas.size(), "end index must be larger than start index");

            std::vector<FieldMeta> new_field_metas;
            for (int idx = start; idx < end; idx++){
                new_field_metas.push_back(_fieldMetas[idx]);
            }
            return SlotMeta(new_field_metas);

        }

        SlotMeta operator() (const std::vector<int>& field_idxs){
            std::vector<FieldMeta> new_field_metas;
            for (auto idx: field_idxs){
                mf_assert(is_valid_idx(idx), "index" + std::to_string(idx) +  " out of range");
                new_field_metas.push_back(_fieldMetas[idx]);
            }
            return SlotMeta(new_field_metas);
        }

        bool operator == (const SlotMeta& rhs){

            bool is_equal = true;
            if (get_num_field_ptr() != rhs.get_num_field_ptr()){
                return false;
            }

            for (int idx = 0; idx < get_num_field_ptr(); idx++){
                if (_fieldMetas[idx] != rhs._fieldMetas[idx]){
                    is_equal = false;
                    break;
                }
            }

            return is_equal;
        }


        SlotMeta operator + (const SlotMeta& rhs) const{
            SlotMeta result(*this);
            result._fieldMetas.insert(
                result._fieldMetas.end(),
                rhs._fieldMetas.begin(),
                rhs._fieldMetas.end());
            return result;
        }

        SlotMeta operator - (const std::vector<std::string>& delete_item) const{

            std::vector<FieldMeta> new_field_metas;
            for (const FieldMeta& field_meta: _fieldMetas){
                if (std::find(delete_item.begin(), delete_item.end(), field_meta._name) == delete_item.end()){
                    new_field_metas.push_back(field_meta);
                }
            }
            return {new_field_metas};

            
        }

        void add_field(const FieldMeta& field_meta){
            assert(!is_there_field(field_meta._name));
            _fieldMetas.push_back(field_meta);
        }

        std::pair<std::vector<int>, std::vector<int>> match_by_name(const SlotMeta& rhs){
            std::vector<int> src_match_idxs;
            std::vector<int> des_match_idxs;

            for (int src_idx = 0; src_idx < rhs.get_num_field_ptr(); src_idx++){
                std::string rhs_field_name = rhs.get_field(src_idx)._name;
                int des_idx = get_idx_ptr(rhs_field_name);
                if (des_idx != -1){
                    src_match_idxs.push_back(src_idx);
                    des_match_idxs.push_back(des_idx);
                }
            }

            return {src_match_idxs, des_match_idxs};
        }




    };


}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTMETA_H