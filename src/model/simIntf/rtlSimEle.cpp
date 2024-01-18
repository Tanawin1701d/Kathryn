//
// Created by tanawin on 18/1/2567.
//

#include "rtlSimEle.h"

namespace kathryn{

    RtlSimEngine::RtlSimEngine() = default;

    //////////////////////////////////////////////////////////
    ///////////////// seq value comp /////////////////////////
    //////////////////////////////////////////////////////////
    seqRtlSimEngine::seqRtlSimEngine(int sz):
            RtlSimEngine(),
            backVal(sz),
            curVal(sz){}

    ValRep& seqRtlSimEngine::getValToCalThisCycle() {
        return backVal;
    }

    //////////////////////////////////////////////////////////
    ///////////////// comb value comp /////////////////////////
    //////////////////////////////////////////////////////////
    combRtlSimEngine::combRtlSimEngine(int sz):
            RtlSimEngine(),
            curVal(sz){}

    ValRep& combRtlSimEngine::getValToCalThisCycle() {
        return curVal;
    }
}
