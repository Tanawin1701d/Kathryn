//
// Created by tanawin on 18/1/2567.
//

#include "rtlSimEle.h"


namespace kathryn{

    RtlSimEngine::RtlSimEngine(int sz):
    backVal(sz),
    curVal(sz){}

    void FlowSimEngine::incUsedTime() {
        amtUsed++;
    }
}

