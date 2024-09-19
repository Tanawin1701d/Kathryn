//
// Created by tanawin on 19/9/2024.
//

#ifndef KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H
#define KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H

#include "kathryn.h"
#include "example/cacheAc/simpleServer/simpleSystem.h"

namespace kathryn::cacheServer{


    constexpr char VCD_FILE_PARAM[]  = "vcdFile";
    constexpr char PROF_FILE_PARAM[] = "profFile";

    class CacheSimItf: public SimInterface{

        SimpleServer& _server;


    public:

        CacheSimItf(PARAM& params, SimpleServer& server):
        SimInterface(1000000,
                     params[VCD_FILE_PARAM],
                     params[PROF_FILE_PARAM],
                     "cacheModel"
                     ),
        _server(server){}

        void describeDef() override{
            //////////////   provide data to memory
        }

        void describeCon() override{


        }

    };


    struct CacheSystemTest{

        void start(PARAM& params){
            SERVER_PARAM serverParam = {{8, 32, OVER_WRITE}, 64, 1};
            mMod(server, SimpleServer, serverParam);
            startModelKathryn();
            //////// start simulator
            CacheSimItf simItf(params, server);
            simItf.simStart();
            resetKathryn();
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_CACHEAC_SIMPLESERVER_UNITTEST_SYSTEMTEST_H
