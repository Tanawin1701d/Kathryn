//
// Created by tanawin on 20/9/2024.
//

#include "systemTest.h"

namespace kathryn::cacheServer{

    void startSimpleCacheAcSim(PARAM& params){
        SERVER_PARAM serverParam = {
                {8, OVER_WRITE, DYNAMIC_FIELD({"value"}, {32})},
                64, 1
        };

        mMod(server, SimpleServer, serverParam);
        startModelKathryn();
        //////// start simulator
        CacheSimItf simItf(params, server);
        simItf.simStart();
        resetKathryn();
    }


}