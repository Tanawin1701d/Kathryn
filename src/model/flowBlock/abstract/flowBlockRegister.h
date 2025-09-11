//
// Created by tanawin on 11/9/25.
//

#ifndef MODEL_FLOWBLOCK_ABSTRACT_FLOWBLOCKREGISTER_H
#define MODEL_FLOWBLOCK_ABSTRACT_FLOWBLOCKREGISTER_H

#include <cassert>

namespace kathryn{


    enum FLOW_BLOCK_TYPE{
        SEQUENTIAL, /** seq to par_no_syn do not reorder it due to controller pattern checking*/
        PARALLEL_AUTO_SYNC,
        PARALLEL_NO_SYN,
        CIF,
        SIF,
        CSELIF,
        CSELSE,
        ZIF,
        ZELIF,
        ZELSE,
        CWHILE,
        SWHILE,
        DOWHILE,
        EXITWHILE,
        CONDWAIT,
        CLKWAIT,
        PIPE_BLOCK,
        PIPE_TRAN,
        PICK,
        PICK_WHEN,
        /** new flow block should be append here*/

        /************************************** **/
        DUMMY_BLOCK,
        FLOW_BLOCK_COUNT

    };

    std::string FBT_to_string(FLOW_BLOCK_TYPE fbt);

}

#endif //MODEL_FLOWBLOCK_ABSTRACT_FLOWBLOCKREGISTER_H