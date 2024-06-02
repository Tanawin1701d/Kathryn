//
// Created by tanawin on 19/1/2567.
//

#ifndef KATHRYN_NUMCONVERT_H
#define KATHRYN_NUMCONVERT_H

#include <string>
#include <vector>
#include <ios>
#include <sstream>
#include "sim/logicRep/valRep.h"

namespace kathryn{

    // class NumConverter {
    //
    // public:
    //
    //     static void addToVec(std::vector<ull> &resultVals) {}
    //
    //     template<typename... Args>
    //     static void addToVec(std::vector<ull> &resultVals, ull valMsb, Args... args) {
    //         resultVals.push_back(valMsb);
    //         addToVec(resultVals, args...);
    //     }
    //
    //     static void checkOverflow(int sz, std::vector<ull>& src){
    //         assert(sz > 0);
    //         int sizeOfValRepElement = sizeof(ull) * 8;
    //         int lastIdx = (sz + sizeOfValRepElement - 1) / sizeOfValRepElement;
    //         lastIdx--;
    //         /**target last*/
    //
    //         if (lastIdx < src.size()){
    //             int startCheckBit = sz % sizeOfValRepElement;
    //             ull ele = src[lastIdx] >> startCheckBit;
    //             assert(ele == 0);
    //         }
    //         for (int idx = lastIdx + 1; idx < src.size(); idx++){
    //             assert(src[idx] == 0);
    //         }
    //
    //     }
    //
    //     template<typename... Args>
    //     static ValRep createValRep(int sz, Args... args) {
    //
    //         /*** collect all value*/
    //         std::vector<ull> values;
    //         addToVec(values, args...);
    //         /**reverse the vector*/
    //         assert(!values.empty());
    //         for (int i = 0; i < (values.size() / 2); i++) {
    //             std::swap(values[i], values[values.size() - 1 - i]);
    //         }
    //         /*** declare new varep*/
    //         assert(sz > 0);
    //         ValRep ret(sz);
    //         checkOverflow(sz, values);
    //         int amtEleToCpy = std::min(values.size(), (size_t)ret.getValArrSize());
    //         for (int i = 0; i < amtEleToCpy; i++) {
    //             ret.getValPtr()[i] = values[i];
    //         }
    //
    //         return ret;
    //     }
    //
    // };
    //
    //     //// gen(1,5) => b11111
    //     //// gen(0,6) => b000000
    //     ValRep genBiConValRep(bool val, int bitSize);

        std::string cvtNum2HexStr(ull x);



}

#endif //KATHRYN_NUMCONVERT_H
