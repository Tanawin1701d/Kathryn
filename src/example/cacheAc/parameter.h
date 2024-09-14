//
// Created by tanawin on 13/9/2024.
//

#ifndef KATHRYN_PARAMETER_H
#define KATHRYN_PARAMETER_H




    namespace kathryn::cacheServer{

        struct KV_PARAM{
            int KEY_SIZE = 8; ///// key size if the cache
            int VALUE_SIZE = 32; //////// value bitwidth

        };

        struct SERVER_PARAM{
            KV_PARAM kvParam;
            int      ingrQueueSize = 8;
            int      amtBank = 1;
            int      amtWordPerBank = 1;

        };


    }



#endif //KATHRYN_PARAMETER_H
