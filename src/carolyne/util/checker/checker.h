//
// Created by tanawin on 28/12/2024.
//

#ifndef KATHRYN_SRC_CAROLYNE_UTIL_CHECKER_CHECKER_H
#define KATHRYN_SRC_CAROLYNE_UTIL_CHECKER_CHECKER_H

#include <iostream>
#include <cassert>
#include "util/termColor/termColor.h"

namespace kathryn{
    namespace carolyne{
        //// carolyne assert
        void crlAss(const bool& cond, const std::string& errorValue);
        //// carolyne verbose
        void crlVb(const std::string& vbStr);
    }
}

#endif //KATHRYN_SRC_CAROLYNE_UTIL_CHECKER_CHECKER_H
