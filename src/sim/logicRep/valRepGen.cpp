//
// Created by tanawin on 6/10/2024.
//
#include<vector>
#include "valRepGen.h"

#include <util/numberic/pmath.h>

#include "util/termColor/termColor.h"

namespace kathryn{

    ValRepBase bin(int sz, const std::string& value){
        return genWithBase(2, sz, value);
    }


    ValRepBase hex(int sz, const std::string& value){
        return genWithBase(16, sz, value);
    }



ValRepBase genWithBase(int baseRange,
                       int sz, const std::string& value){

    assert(baseRange > 1);

    int baseBitSize = log2Ceil(baseRange);

    std::vector<int> convertedChar;
    for (char v: value){
        char lv = std::tolower(v, std::locale());
        int  convertedIv = 0;
        if (lv >= '0' && lv <= '9'){
            convertedIv = lv - '0';
        }else if (lv >= 'a' && lv <= 'z'){
            convertedIv = lv - 'a' + 10;
        }
        if (convertedIv < 0 || convertedIv >= baseRange){
            std::cout << TC_RED << "value overflow" << TC_DEF << std::endl;
            assert(false);
        }
        convertedChar.push_back(convertedIv);
    }

    ///////// reverse the vector
    std::reverse(convertedChar.begin(), convertedChar.end());
    assert( (convertedChar.size() * baseBitSize) <= sz );

    ///////// map to the size of ull
    std::vector<ull> result; //// first index is lowest value
    int hopSize = bitSizeOfUll/baseBitSize;
    assert((bitSizeOfUll % baseBitSize) == 0);

    ///////// create data
    int idx = 0;
    while (idx < convertedChar.size()){
        ull curValue = 0;
        int subIdx   = 0;
        for (; subIdx < hopSize; subIdx++ ){
            if ( (idx + subIdx) >= convertedChar.size()){break;}
            curValue = curValue | (((ull)convertedChar[idx + subIdx]) << (subIdx * baseBitSize));
        }
        idx += subIdx;
        result.push_back( curValue);
    }

    //////// create  valRepBase
    int bitPerByte = 8;

    int amtByte        = (sz + (bitPerByte-1)) / bitPerByte;
    int continLength   = (amtByte + sizeof(ull)-1)  / sizeof(ull);
    int amtAlignedByte = 1 << log2Ceil(amtByte); ///// at double scale

    ull* dayta = new ull[result.size()];
    std::copy(result.begin(), result.end(), dayta);
    ValRepBase ret(std::min(amtAlignedByte, 8), dayta);

    if (continLength > 1){ ret.setContinLength(continLength);}
    ret.setSize(sz);

    return ret;

}



}
