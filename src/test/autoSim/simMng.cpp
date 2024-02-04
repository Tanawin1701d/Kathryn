//
// Created by tanawin on 4/2/2567.
//

#include "simMng.h"
#include "util/termColor/termColor.h"


namespace kathryn{

    std::vector<SimAutoInterface*>* testPool = nullptr;


    void addSimTestToPool(SimAutoInterface* simEle){
        if (testPool == nullptr){
            testPool = new std::vector<SimAutoInterface*>;
        }
        assert(simEle != nullptr);
        testPool->push_back(simEle);
    }

    void startAutoSimTest(){

        std::cout << TC_BLUE << "[kathryn auto test] start auto test\n" << TC_DEF;

        if (testPool == nullptr){
            std::cout << "[kathryn auto test] " << "auto sim has nothing to simulate.\n";
        }

        int testCase = 0;
        for (auto sif: *testPool){
            std::cout << TC_BLUE << "[kathryn auto test] " << "start sim testcase "<< testCase << TC_DEF <<"\n";
            sif->simStart();
            std::cout << TC_BLUE << "[kathryn auto test] " << "finnish sim testcase "<< testCase << TC_DEF <<"\n";
        }
    }









}