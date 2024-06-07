//
// Created by tanawin on 25/1/2567.
//

#include "numConvert.h"

namespace kathryn{

    ull genBiConValRep(bool val, int bitSize){
        const int MAX_BIT = 64;
        assert(bitSize <= MAX_BIT);
        if (val)
            return (bitSize < MAX_BIT) ? (1 << bitSize) : -1;
        else
            return 0;
    }

    std::string cvtNum2HexStr(ull x){
        std::ostringstream oss;
        oss << std::hex <<std::uppercase << ull(x);
        return oss.str();
    }

}