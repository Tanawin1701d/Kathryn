//
// Created by tanawin on 19/1/2567.
//

#ifndef KATHRYN_NUMCONVERT_H
#define KATHRYN_NUMCONVERT_H

#include <string>
#include <vector>
#include "sim/logicRep/valRep.h"

namespace kathryn{

    class NumConverter {

    public:

        static void addToVec(std::vector<ull> &resultVals) {}

        template<typename... Args>
        static void addToVec(std::vector<ull> &resultVals, ull valMsb, Args... args) {
            resultVals.push_back(valMsb);
            addToVec(resultVals, args...);
        }

        template<typename... Args>
        static ValRep cvtStrToValRep(int sz, Args... args) {

            /*** collect all value*/
            std::vector<ull> values;
            addToVec(values, args...);
            /**reverse the vector*/
            assert(!values.empty());
            for (int i = 0; i < (values.size() / 2); i++) {
                std::swap(values[i], values[values.size() - 1 - i]);
            }
            /*** declare new varep*/
            assert(sz > 0);
            ValRep ret(sz);
            assert(ret.getValArrSize() >= values.size());
            for (int i = 0; i < values.size(); i++) {
                ret.getVal()[i] = values[i];
            }
            ret.fillZeroToValrep(ret.getLen());

            return ret;
        }

    };

        //// gen(1,5) => b11111
        //// gen(0,6) => b000000
        ValRep genBiConValRep(bool val, int bitSize);



}

#endif //KATHRYN_NUMCONVERT_H
