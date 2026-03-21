//
// Created by tanawin on 6/10/2024.
//
#include "vector"
#include "val_rep_gen.h"

#include "util/numberic/pmath.h"

#include "util/term_color/term_color.h"

namespace kathryn{

    ValRepBase bin(int sz, const std::string& value){
        return gen_with_base(2, sz, value);
    }


    ValRepBase hex(int sz, const std::string& value){
        return gen_with_base(16, sz, value);
    }



ValRepBase gen_with_base(int base_range,
                       int sz, const std::string& value){

    assert(base_range > 1);

    int base_bit_size = log2Ceil(base_range);

    std::vector<int> converted_char;
    for (char v: value){
        char lv = std::tolower(v, std::locale());
        int  converted_iv = 0;
        if (lv >= '0' && lv <= '9'){
            converted_iv = lv - '0';
        }else if (lv >= 'a' && lv <= 'z'){
            converted_iv = lv - 'a' + 10;
        }
        if (converted_iv < 0 || converted_iv >= base_range){
            std::cout << TC_RED << "value overflow" << TC_DEF << std::endl;
            assert(false);
        }
        converted_char.push_back(converted_iv);
    }

    ///////// reverse the vector
    std::reverse(converted_char.begin(), converted_char.end());
    assert( (converted_char.size() * base_bit_size) <= sz );

    ///////// map to the size of ull
    std::vector<ull> result; //// first index is lowest value
    int hop_size = bit_size_of_ull/base_bit_size;
    assert((bit_size_of_ull % base_bit_size) == 0);

    ///////// create data
    int idx = 0;
    while (idx < converted_char.size()){
        ull cur_value = 0;
        int sub_idx   = 0;
        for (; sub_idx < hop_size; sub_idx++ ){
            if ( (idx + sub_idx) >= converted_char.size()){break;}
            cur_value = cur_value | (((ull)converted_char[idx + sub_idx]) << (sub_idx * base_bit_size));
        }
        idx += sub_idx;
        result.push_back( cur_value);
    }

    //////// create  val_rep_base
    int bit_per_byte = 8;

    int amt_byte        = (sz + (bit_per_byte-1)) / bit_per_byte;
    int contin_length   = (amt_byte + sizeof(ull)-1)  / sizeof(ull);
    int amt_aligned_byte = 1 << log2Ceil(amt_byte); ///// at double scale

    ull* dayta = new ull[result.size()];
    std::copy(result.begin(), result.end(), dayta);
    ValRepBase ret(std::min(amt_aligned_byte, 8), dayta);

    if (contin_length > 1){ ret.set_contin_length(contin_length);}
    ret.set_size(sz);

    return ret;

}



}
