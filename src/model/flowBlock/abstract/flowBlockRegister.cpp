//
// Created by tanawin on 11/9/25.
//

#include "flowBlockRegister.h"


namespace kathryn{
    std::string FBT_to_string(FLOW_BLOCK_TYPE fbt){
        std::string mapper[FLOW_BLOCK_COUNT] = {
            "SEQUENTIAL"         ,
            "PARALLEL_AUTO_SYNC" ,
            "PARALLEL_NO_SYN"    ,
            "CIF"                ,
            "SIF"                ,
            "CSELIF"             ,
            "CSELSE"             ,
            "ZIF"                ,
            "ZELIF"              ,
            "ZELSE"              ,
            "CWHILE"             ,
            "SWHILE"             ,
            "DOWHILE"            ,
            "EXITWHILE"          ,
            "CONDWAIT"           ,
            "CLKWAIT"            ,
            "PIPE_BLOCK"         ,
            "PIPE_TRAN"          ,
            "PICK"               ,
            "PICK_WHEN"          ,
            "PSUEDO_BLOCK"       ,
            "DUMMY_BLOCK"
        };
        assert(fbt < FLOW_BLOCK_COUNT);
        return mapper[fbt];
    }
}
