//
// Created by tanawin on 4/2/2567.
//

#include "sim_mng.h"



namespace kathryn{



    bool arrange_test_cmp(AutoTestEle* lhs, AutoTestEle* rhs){
        assert(lhs != nullptr);
        assert(rhs != nullptr);
        return lhs->get_sim_id() < rhs->get_sim_id();
    }

    std::vector<AutoTestEle*>* test_pool = nullptr;

    void add_sim_test_to_pool(AutoTestEle* sim_ele){
        if (test_pool == nullptr){
            test_pool = new std::vector<AutoTestEle*>;
        }
        assert(sim_ele != nullptr);
        test_pool->push_back(sim_ele);
    }




    void start_auto_sim_test(PARAM& params){

        std::cout << TC_BLUE << "[kathryn auto test] start auto test\n" << TC_DEF;

        if (test_pool == nullptr){
            std::cout << "[kathryn auto test] " << "auto sim has nothing to simulate.\n";
        }

        std::sort(test_pool->begin(), test_pool->end(), arrange_test_cmp);

        assert(params.find("prefix") != params.end());
        std::string prefix_path = params["prefix"];

        int test_case = 1;
        for (auto sif: *test_pool){
            std::cout << TC_BLUE << "[kathryn auto test] " << "start sim testcase "<< test_case << " id: " << sif->get_sim_id()<< TC_DEF <<"\n";
            sif->start(prefix_path, get_spbm(params));
            reset_kathryn();
            std::cout << TC_BLUE << "[kathryn auto test] " << "finnish sim testcase "<< test_case << " id: " << sif->get_sim_id()  << TC_DEF <<"\n";
            test_case++;
            std::cout << TC_BLUE << "----------------------" << TC_DEF << std::endl;
            ////break;
        }

    }









}