//
// Created by tanawin on 4/2/2567.
//

#include "simMng.h"



namespace kathryn{

    std::vector<SimAutoInterface*>* testPool = nullptr;
    std::unordered_set<int> addedSimAutoInterface;


    void addSimTestToPool(SimAutoInterface* simEle){
        if (testPool == nullptr){
            testPool = new std::vector<SimAutoInterface*>;
        }
        assert(simEle != nullptr);
        if (addedSimAutoInterface.find(simEle->getSimId()) == addedSimAutoInterface.end())
            return;
        testPool->push_back(simEle);
    }


    bool arrageSimInterfaceCmp(const SimAutoInterface* lhs,
                               const SimAutoInterface* rhs
                               ){
        return lhs->getSimId() <= rhs->getSimId();
    }

    void startAutoSimTest(){

        std::cout << TC_BLUE << "[kathryn auto test] start auto test\n" << TC_DEF;

        if (testPool == nullptr){
            std::cout << "[kathryn auto test] " << "auto sim has nothing to simulate.\n";
        }

        std::sort(testPool->begin(), testPool->end(), arrageSimInterfaceCmp);

        int testCase = 0;
        for (auto sif: *testPool){
            std::cout << TC_BLUE << "[kathryn auto test] " << "start sim testcase "<< testCase << " id: " << sif->getSimId()<< TC_DEF <<"\n";
            sif->simStart();
            std::cout << TC_BLUE << "[kathryn auto test] " << "finnish sim testcase "<< testCase << " id: " << sif->getSimId()  << TC_DEF <<"\n";
            testCase++;
        }

    }









}