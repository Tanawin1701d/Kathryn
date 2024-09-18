//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_PARAMETER_H
#define KATHRYN_PARAMETER_H




    namespace kathryn::cacheServer{

        enum REPLACE_POL{
            OVER_WRITE,
            AVOID_CONFLICT
        };

        struct KV_PARAM{
            int KEY_SIZE = 8; ///// key size if the cache
            int VALUE_SIZE = 32; //////// value bitwidth
            REPLACE_POL replacePol;


        };

        struct SERVER_PARAM{
            KV_PARAM kvParam;
            int      ingrQueueSize = 8;
            int      prefixBit     = 2; ////// it mean key bit is set to 6 == 64 element perbank
            ////// prefix bit must lower than key size

        };


    }



#endif //KATHRYN_PARAMETER_H
