//
// Created by tanawin on 19/1/2567.
//

#ifndef KATHRYN_NUMCONVERT_H
#define KATHRYN_NUMCONVERT_H

#include "string"
#include "vector"
#include "ios"
#include "sstream"
#include "sim/logic_rep/val_rep.h"

namespace kathryn{

    // class NumConverter {
    //
    // public:
    //
    //     static void add_to_vec(std::vector<ull> &result_vals) {}
    //
    //     template<typename... Args>
    //     static void add_to_vec(std::vector<ull> &result_vals, ull val_msb, Args... args) {
    //         result_vals.push_back(val_msb);
    //         add_to_vec(result_vals, args...);
    //     }
    //
    //     static void check_overflow(int sz, std::vector<ull>& src){
    //         assert(sz > 0);
    //         int size_of_val_rep_element = sizeof(ull) * 8;
    //         int last_idx = (sz + size_of_val_rep_element - 1) / size_of_val_rep_element;
    //         last_idx--;
    //         /**target last*/
    //
    //         if (last_idx < src.size()){
    //             int start_check_bit = sz % size_of_val_rep_element;
    //             ull ele = src[last_idx] >> start_check_bit;
    //             assert(ele == 0);
    //         }
    //         for (int idx = last_idx + 1; idx < src.size(); idx++){
    //             assert(src[idx] == 0);
    //         }
    //
    //     }
    //
    //     template<typename... Args>
    //     static ValRep create_val_rep(int sz, Args... args) {
    //
    //         /*** collect all value*/
    //         std::vector<ull> values;
    //         add_to_vec(values, args...);
    //         /**reverse the vector*/
    //         assert(!values.empty());
    //         for (int i = 0; i < (values.size() / 2); i++) {
    //             std::swap(values[i], values[values.size() - 1 - i]);
    //         }
    //         /*** declare new varep*/
    //         assert(sz > 0);
    //         ValRep ret(sz);
    //         check_overflow(sz, values);
    //         int amt_ele_to_cpy = std::min(values.size(), (size_t)ret.get_val_arr_size());
    //         for (int i = 0; i < amt_ele_to_cpy; i++) {
    //             ret.get_val_ptr()[i] = values[i];
    //         }
    //
    //         return ret;
    //     }
    //
    // };
    //
    //     //// gen(1,5) => b11111
    //     //// gen(0,6) => b000000
    ull gen_bi_con_val_rep(bool val, int bit_size);
    std::string cvt_num2_hex_str(ull x);
    std::string cvt_num2_bin_str(ull x);



}

#endif //KATHRYN_NUMCONVERT_H
