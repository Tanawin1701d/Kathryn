//
// Created by tanawin on 7/9/2024.
//

#ifndef SIMDEBUGGER_H
#define SIMDEBUGGER_H

#include "iostream"

namespace kathryn{

    void smAssert(bool valid,const std::string& msg);
    void smWarn(bool valid, const std::string& msg);

}

#endif //SIMDEBUGGER_H
