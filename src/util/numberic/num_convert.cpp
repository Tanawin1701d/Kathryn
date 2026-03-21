//
// Created by tanawin on 25/1/2567.
//

#include "num_convert.h"

namespace kathryn{

    ull gen_bi_con_val_rep(bool val, int bit_size){
        const int MAX_BIT = 64;
        assert(bit_size <= MAX_BIT);
        if (val)
            return (bit_size < MAX_BIT) ? ((((ull)1) << bit_size) - 1) : -1;
        else
            return 0;
    }

    std::string cvt_num2_hex_str(ull x){
        std::ostringstream oss;
        oss << std::hex <<std::uppercase << ull(x);
        return "0x" + oss.str();
    }

    std::string cvt_num2_bin_str(ull x){
        std::bitset<64> b(x);
        std::string bin_str = b.to_string();

        // Remove leading zeros
        size_t first_one = bin_str.find('1');
        if (first_one != std::string::npos) {
            bin_str = bin_str.substr(first_one);
        } else {
            bin_str = "0";  // Handle the case when x is 0
        }

        return "0b" + bin_str;
    }


}