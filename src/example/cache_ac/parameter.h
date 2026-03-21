//
// Created by tanawin on 20/9/2024.
//

#ifndef KATHRYN_SRC_EXAMPLE_CACHEAC_PARAMETER_H
#define KATHRYN_SRC_EXAMPLE_CACHEAC_PARAMETER_H


#include "iostream"
#include "example/data_struct/field/dynamic_field.h"

namespace kathryn::cache_server{

    enum REPLACE_POL{
        OVER_WRITE,
        AVOID_CONFLICT
    };

    struct KV_PARAM{
        int KEY_SIZE = 8; ///// key size if the cache
        REPLACE_POL replace_pol = OVER_WRITE;
        DYNAMIC_FIELD valuefield;
    };

    struct SERVER_PARAM{
        KV_PARAM kv_param;
        int      ingr_queue_size = 8;
        int      prefix_bit     = 2; ////// it mean key bit is set to 6 == 64 element perbank
        ////// prefix bit must lower than key size
    };

    struct KV_DEBUG{
        std::string key;
        std::string value;
    };

}

#endif //KATHRYN_SRC_EXAMPLE_CACHEAC_PARAMETER_H
