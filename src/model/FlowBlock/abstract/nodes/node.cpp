//
// Created by tanawin on 8/1/2567.
//
#include"node.h"

namespace kathryn{


    std::string NT_to_string(NODE_TYPE nt){

        std::string mapper[NODE_TYPE_CNT] = {
        "ASM_NODE",
        "STATE_NODE",
        "SYN_NODE",
        "PSEUDO_NODE",
        "DUMMY_NODE",
        "START_NODE",
        "WAITCOND_NODE",
        "WAITCYCLE_NODE"
        };
        return mapper[nt];

    }






}