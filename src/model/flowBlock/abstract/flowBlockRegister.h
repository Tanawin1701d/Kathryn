//
// Created by tanawin on 11/9/25.
//

#ifndef MODEL_FLOWBLOCK_ABSTRACT_FLOWBLOCKREGISTER_H
#define MODEL_FLOWBLOCK_ABSTRACT_FLOWBLOCKREGISTER_H

#include <cassert>
#include <string>

namespace kathryn{


    enum FLOW_BLOCK_TYPE{
        SEQUENTIAL          = 0, /** seq to par_no_syn do not reorder it due to controller pattern checking*/
        PARALLEL_AUTO_SYNC  = 1,
        PARALLEL_NO_SYN     = 2,
        CIF                 = 3,
        SIF                 = 4,
        CSELIF              = 5,
        CSELSE              = 6,
        ZIF                 = 7,
        ZELIF               = 8,
        ZELSE               = 9,
        CWHILE              = 10,
        SWHILE              = 11,
        DOWHILE             = 12,
        EXITWHILE           = 13,
        CONDWAIT            = 14,
        CLKWAIT             = 15,
        PIPE_BLOCK          = 16,
        PIPE_TRAN           = 17,
        PICK                = 18,
        PICK_WHEN           = 19,
        PSUEDO_BLOCK        = 20,
        /** new flow block should be append here*/

        /************************************** **/
        DUMMY_BLOCK         = 21,
        FLOW_BLOCK_COUNT    = 22
    };

    std::string FBT_to_string(FLOW_BLOCK_TYPE fbt);

}

#endif //MODEL_FLOWBLOCK_ABSTRACT_FLOWBLOCKREGISTER_H