//
// Created by tanawin on 25/1/2567.
//

#include "numConvert.h"

namespace kathryn{

    ull genBiConValRep(bool val, int bitSize){
        const int MAX_BIT = 64;
        assert(bitSize <= MAX_BIT);
        if (val)
            return (bitSize < MAX_BIT) ? ((((ull)1) << bitSize) - 1) : -1;
        else
            return 0;
    }

    std::string cvtNum2HexStr(ull x){
        std::ostringstream oss;
        oss << std::hex <<std::uppercase << ull(x);
        return "0x" + oss.str();
    }

    std::string cvtNum2BinStr(ull x){
        std::bitset<64> b(x);
        std::string binStr = b.to_string();

        // Remove leading zeros
        size_t firstOne = binStr.find('1');
        if (firstOne != std::string::npos) {
            binStr = binStr.substr(firstOne);
        } else {
            binStr = "0";  // Handle the case when x is 0
        }

        return "0b" + binStr;
    }


}