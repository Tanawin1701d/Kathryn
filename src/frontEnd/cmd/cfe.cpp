//
// Created by tanawin on 4/2/2567.
//

#include "cassert"
#include "utility"
#include "cfe.h"

#include "example/riscv/generation/riscvGen.h"
#include "example/riscv/simulation/riscvSim.h"
#include "example/riscv/simulation/riscvSimSort.h"
#include "test/autoSim/simMng.h"
#include "test/autoGen/genMng.h"
#include "example/o3/simulation/o3Sim.h"
#include "example/o3/simCompare/ctrl/simCtrlKride.h"
#include "example/o3/generation/o3Gen.h"


#ifdef BUILD_RIDECORE
#include "example/o3/simCompare/ctrl/simCtrlComb.h"
#include "simCtrlRide.h"
#endif

namespace kathryn{


    void printWelcomeScreen(){

        std::cout << "------------------------------------------------------------------------\n"
                     "-------- \n"
                     "-------- KATHRYN hardware construction framework with hybrid design flow\n"
                     "-------- \n"
                 <<  "-------- program compiled on " << __DATE__ << " at " << __TIME__ << "\n"
                     "-------- \n"
                     "------------------------------------------------------------------------\n";

    }

    void testSimple(PARAM& params){
            startAutoSimTest(params);
    }

    void testRiscv(PARAM& params){
        riscv::RISCV_MNG riscTestMng;
        riscTestMng.start(params);
        std::cout << TC_GREEN <<  " finish rv sim " << TC_DEF << std::endl;
        /////////////////delete x;

    }

    void testRiscvSort(PARAM& params){
        riscv::RISCV_SORT_MNG riscTestMng;
        riscTestMng.start(params);
        std::cout << TC_GREEN <<  " finish rv [sort] sim " << TC_DEF << std::endl;
    }

    void testRiscvGen(PARAM& params){
        riscv::RISCV_GEN_MNG riscGenMng;
        riscGenMng.startGen(params);
        std::cout << TC_GREEN << "finish rv [gen]" << TC_DEF << std::endl;
    }

    // void test_cacheAc_sim(PARAM& params){
    //     cacheServer::startSimpleCacheAcSim(params);
    //     std::cout << TC_GREEN << "finish cacheSimple sim [sim]" << TC_DEF << std::endl;
    // }

    void testO3Sim(PARAM& params){
        o3::O3_MNG o3SimMng;
        o3SimMng.start(params);
        std::cout << TC_GREEN << "finish o3 sim [sim]" << TC_DEF << std::endl;
    }

    void testKrideSim(PARAM& params){
        
        o3::KRIDE_MNG krideSimMng;
        krideSimMng.start(params);
        std::cout << TC_GREEN << "finish kride sim [sim]" << TC_DEF << std::endl;

    }

    void testRideSim(PARAM& params){

#ifdef BUILD_RIDECORE
        o3::RIDE_MNG rideSimMng;
        rideSimMng.start(params);
        std::cout << TC_GREEN << "finish ride sim [sim]" << TC_DEF << std::endl;
#else
        std::cout << "RIDE simulation is not enabled. Please build with BUILD_RIDECORE=ON" << std::endl;
#endif

    }

    void testCombKrideRide(PARAM& params){

#ifdef BUILD_RIDECORE
        o3::COMB_MNG combSimMng;
        combSimMng.start(params);
        std::cout << TC_GREEN << "finish comb kride ride sim [sim]" << TC_DEF << std::endl;
#else
        std::cout << "RIDE simulation is not enabled. Please build with BUILD_RIDECORE=ON" << std::endl;
#endif


    }

    void testO3Gen(PARAM& params){
        o3::O3_GEN_MNG o3Gen;
        o3Gen.startGen(params);
        std::cout << TC_GREEN << "finish O3 [gen]" << TC_DEF << std::endl;
    }





    void start(PARAM& params) {

        printWelcomeScreen();

        if (params["ioOptimize"] == "true") {
            std::iosBase::syncWithStdio(false);
            std::cin.tie(nullptr);
        }

        if (params["testType"] == "testSimple") {
            testSimple(params);
        } else if (params["testType"] == "testRiscv") {
            testRiscv(params);
        } else if (params["testType"] == "testRiscvSort"){
            testRiscvSort(params);
        }else if (params["testType"] == "testGenRiscv"){
            testRiscvGen(params);
        }else if (params["testType"]  == "testGen"){
            startGenEle(params);
        }else if (params["testType"]  == "testSimpleCacheAcc"){
            //test_cacheAc_sim(params);
        }else if (params["testType"]  == "testO3Sim"){
            testO3Sim(params);
        }else if (params["testType"] == "testKrideSim"){
            testKrideSim(params);
        }else if (params["testType"] == "testRideSim"){
            testRideSim(params);
        }else if (params["testType"] == "testKrideRideCombSim"){
            testCombKrideRide(params);
        }else if (params["testType"] == "testGenO3"){
            testO3Gen(params);
        }else{
            std::cout << "there is no command to test system" << std::endl;
        }

        std::cout << "[kathryn] exit program" << std::endl;
    }



}
