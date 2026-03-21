//
// Created by tanawin on 4/2/2567.
//

#include "cassert"
#include "utility"
#include "cfe.h"

#include "example/riscv/generation/riscv_gen.h"
#include "example/riscv/simulation/riscv_sim.h"
#include "example/riscv/simulation/riscv_sim_sort.h"
#include "test/auto_sim/sim_mng.h"
#include "test/auto_gen/gen_mng.h"
#include "example/o3/simulation/o3_sim.h"
#include "example/o3/sim_compare/ctrl/sim_ctrl_kride.h"
#include "example/o3/generation/o3_gen.h"


#ifdef BUILD_RIDECORE
#include "example/o3/sim_compare/ctrl/sim_ctrl_comb.h"
#include "sim_ctrl_ride.h"
#endif

namespace kathryn{


    void print_welcome_screen(){

        std::cout << "------------------------------------------------------------------------\n"
                     "-------- \n"
                     "-------- KATHRYN hardware construction framework with hybrid design flow\n"
                     "-------- \n"
                 <<  "-------- program compiled on " << __DATE__ << " at " << __TIME__ << "\n"
                     "-------- \n"
                     "------------------------------------------------------------------------\n";

    }

    void test_simple(PARAM& params){
            start_auto_sim_test(params);
    }

    void test_riscv(PARAM& params){
        riscv::RISCV_MNG risc_test_mng;
        risc_test_mng.start(params);
        std::cout << TC_GREEN <<  " finish rv sim " << TC_DEF << std::endl;
        /////////////////delete x;

    }

    void test_riscv_sort(PARAM& params){
        riscv::RISCV_SORT_MNG risc_test_mng;
        risc_test_mng.start(params);
        std::cout << TC_GREEN <<  " finish rv [sort] sim " << TC_DEF << std::endl;
    }

    void test_riscv_gen(PARAM& params){
        riscv::RISCV_GEN_MNG risc_gen_mng;
        risc_gen_mng.start_gen(params);
        std::cout << TC_GREEN << "finish rv [gen]" << TC_DEF << std::endl;
    }

    // void test_cacheAc_sim(PARAM& params){
    //     cache_server::start_simple_cache_ac_sim(params);
    //     std::cout << TC_GREEN << "finish cache_simple sim [sim]" << TC_DEF << std::endl;
    // }

    void test_o3_sim(PARAM& params){
        o3::O3_MNG o3SimMng;
        o3SimMng.start(params);
        std::cout << TC_GREEN << "finish o3 sim [sim]" << TC_DEF << std::endl;
    }

    void test_kride_sim(PARAM& params){
        
        o3::KRIDE_MNG kride_sim_mng;
        kride_sim_mng.start(params);
        std::cout << TC_GREEN << "finish kride sim [sim]" << TC_DEF << std::endl;

    }

    void test_ride_sim(PARAM& params){

#ifdef BUILD_RIDECORE
        o3::RIDE_MNG ride_sim_mng;
        ride_sim_mng.start(params);
        std::cout << TC_GREEN << "finish ride sim [sim]" << TC_DEF << std::endl;
#else
        std::cout << "RIDE simulation is not enabled. Please build with BUILD_RIDECORE=ON" << std::endl;
#endif

    }

    void test_comb_kride_ride(PARAM& params){

#ifdef BUILD_RIDECORE
        o3::COMB_MNG comb_sim_mng;
        comb_sim_mng.start(params);
        std::cout << TC_GREEN << "finish comb kride ride sim [sim]" << TC_DEF << std::endl;
#else
        std::cout << "RIDE simulation is not enabled. Please build with BUILD_RIDECORE=ON" << std::endl;
#endif


    }

    void test_o3_gen(PARAM& params){
        o3::O3_GEN_MNG o3Gen;
        o3Gen.start_gen(params);
        std::cout << TC_GREEN << "finish O3 [gen]" << TC_DEF << std::endl;
    }





    void start(PARAM& params) {

        print_welcome_screen();

        if (params["io_optimize"] == "true") {
            std::ios_base::sync_with_stdio(false);
            std::cin.tie(nullptr);
        }

        if (params["test_type"] == "test_simple") {
            test_simple(params);
        } else if (params["test_type"] == "test_riscv") {
            test_riscv(params);
        } else if (params["test_type"] == "test_riscv_sort"){
            test_riscv_sort(params);
        }else if (params["test_type"] == "test_gen_riscv"){
            test_riscv_gen(params);
        }else if (params["test_type"]  == "test_gen"){
            start_gen_ele(params);
        }else if (params["test_type"]  == "test_simple_cache_acc"){
            //test_cacheAc_sim(params);
        }else if (params["test_type"]  == "test_o3_sim"){
            test_o3_sim(params);
        }else if (params["test_type"] == "test_kride_sim"){
            test_kride_sim(params);
        }else if (params["test_type"] == "test_ride_sim"){
            test_ride_sim(params);
        }else if (params["test_type"] == "test_kride_ride_comb_sim"){
            test_comb_kride_ride(params);
        }else if (params["test_type"] == "test_gen_o3"){
            test_o3_gen(params);
        }else{
            std::cout << "there is no command to test system" << std::endl;
        }

        std::cout << "[kathryn] exit program" << std::endl;
    }



}
