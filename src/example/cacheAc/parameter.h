//
// Created by tanawin on 20/9/2024.
//

#ifndef KATHRYN_SRC_EXAMPLE_CACHEAC_PARAMETER_H
#define KATHRYN_SRC_EXAMPLE_CACHEAC_PARAMETER_H


#include<iostream>

namespace kathryn::cacheServer{

    enum REPLACE_POL{
        OVER_WRITE,
        AVOID_CONFLICT
    };

    struct KV_PARAM{
        int KEY_SIZE = 8; ///// key size if the cache
        int VALUE_SIZE = 32; //////// value bitwidth
        REPLACE_POL replacePol = OVER_WRITE;
    };

    struct SERVER_PARAM{
        KV_PARAM kvParam;
        int      ingrQueueSize = 8;
        int      prefixBit     = 2; ////// it mean key bit is set to 6 == 64 element perbank
        ////// prefix bit must lower than key size
    };

    struct KV_DEBUG{
        std::string key;
        std::string value;
    };

}

#endif //KATHRYN_SRC_EXAMPLE_CACHEAC_PARAMETER_H
