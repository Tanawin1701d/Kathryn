//
// Created by tanawin on 4/2/2567.
//

#include <cassert>
#include <utility>
#include "cfe.h"
#include "test/autoSim/simMng.h"
#include "example/riscv/simulation/RISCV_sim_sort.h"
#include "test/autoGen/genMng.h"


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
        }else if (params["testType"]  == "testGen"){
            startGenEle(params);
        }else{
            std::cout << "there is no command to test system" << std::endl;
        }

        std::cout << "[kathryn] exit program" << std::endl;
    }



}
