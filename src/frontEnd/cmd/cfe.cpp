//
// Created by tanawin on 4/2/2567.
//

#include <cassert>
#include <utility>
#include "cfe.h"

#include "example/riscv/generation/RISCV_gen.h"
#include "example/cacheAc/simpleServer/unitTest/systemTest.h"
#include "example/riscv/simulation/RISCV_sim.h"
#include "example/riscv/simulation/RISCV_sim_sort.h"
#include "test/autoSim/simMng.h"
#include "test/autoGen/genMng.h"
#include "example/o3/simulation/o3_sim.h"
#include "example/o3/simCompare/ctrl/simCtrlKride.h"
#include "example/o3/simCompare/ctrl/simCtrlComb.h"


///#ifdef BUILD_EXT_SIM
#include "simCtrlRide.h"
#include "example/o3/simCompare/ctrl/simCtrlCombCxx.h"
///#endif


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

    void test_simple(PARAM& params){
            startAutoSimTest(params);
    }

    void test_riscv(PARAM& params){
        riscv::RISCV_MNG riscTestMng;
        riscTestMng.start(params);
        std::cout << TC_GREEN <<  " finish rv sim " << TC_DEF << std::endl;
        /////////////////delete x;

    }

    void test_riscv_sort(PARAM& params){
        riscv::RISCV_SORT_MNG riscTestMng;
        riscTestMng.start(params);
        std::cout << TC_GREEN <<  " finish rv [sort] sim " << TC_DEF << std::endl;
    }

    void test_riscv_gen(PARAM& params){
        riscv::RISCV_GEN_MNG riscGenMng;
        riscGenMng.startGen(params);
        std::cout << TC_GREEN << "finish rv [gen]" << TC_DEF << std::endl;
    }

    // void test_cacheAc_sim(PARAM& params){
    //     cacheServer::startSimpleCacheAcSim(params);
    //     std::cout << TC_GREEN << "finish cacheSimple sim [sim]" << TC_DEF << std::endl;
    // }

    void test_o3_sim(PARAM& params){
        o3::O3_MNG o3SimMng;
        o3SimMng.start(params);
        std::cout << TC_GREEN << "finish o3 sim [sim]" << TC_DEF << std::endl;
    }

    void test_kride_sim(PARAM& params){
        o3::KRIDE_MNG krideSimMng;
        krideSimMng.start(params);
        std::cout << TC_GREEN << "finish kride sim [sim]" << TC_DEF << std::endl;

    }

    void test_ride_sim(PARAM& params){

////#ifdef BUILD_EXT_SIM

        o3::RIDE_MNG rideSimMng;
        rideSimMng.start(params);
        std::cout << TC_GREEN << "finish ride sim [sim]" << TC_DEF << std::endl;


////#endif

    }

    void test_comb_kride_ride(PARAM& params){
        o3::COMB_MNG combSimMng;
        combSimMng.start(params);
        std::cout << TC_GREEN << "finish comb kride ride sim [sim]" << TC_DEF << std::endl;
    }

    void test_comb_kride_ride_cxx(PARAM& params){

        o3::COMB_CXX_MNG combSimMng;
        combSimMng.start(params);
        std::cout << TC_GREEN << "finish comb kride ride sim [sim]" << TC_DEF << std::endl;

    }


    void start(PARAM& params) {

        printWelcomeScreen();

        if (params["ioOptimize"] == "true") {
            std::ios_base::sync_with_stdio(false);
            std::cin.tie(nullptr);
        }

        if (params["testType"] == "testSimple") {
            test_simple(params);
        } else if (params["testType"] == "testRiscv") {
            test_riscv(params);
        } else if (params["testType"] == "testRiscvSort"){
            test_riscv_sort(params);
        }else if (params["testType"] == "testGenRiscv"){
            test_riscv_gen(params);
        }else if (params["testType"]  == "testGen"){
            startGenEle(params);
        }else if (params["testType"]  == "testSimpleCacheAcc"){
            //test_cacheAc_sim(params);
        }else if (params["testType"]  == "testO3Sim"){
            test_o3_sim(params);
        }else if (params["testType"] == "testKrideSim"){
            test_kride_sim(params);
        }else if (params["testType"] == "testRideSim"){
            test_ride_sim(params);
        }else if (params["testType"] == "testKrideRideCombSim"){
            test_comb_kride_ride(params);
        }else if (params["testType"] == "testKrideRideCombCxxSim"){
            test_comb_kride_ride_cxx(params);
        }else{
            std::cout << "there is no command to test system" << std::endl;
        }

        std::cout << "[kathryn] exit program" << std::endl;
    }



}
