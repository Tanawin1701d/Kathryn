//
// Created by tanawin on 4/2/2567.
//

#include "simMng.h"



namespace kathryn{



    bool arrangeTestCmp(AutoTestEle* lhs, AutoTestEle* rhs){
        assert(lhs != nullptr);
        assert(rhs != nullptr);
        return lhs->getSimId() < rhs->getSimId();
    }

    std::vector<AutoTestEle*>* testPool = nullptr;

    void addSimTestToPool(AutoTestEle* simEle){
        if (testPool == nullptr){
            testPool = new std::vector<AutoTestEle*>;
        }
        assert(simEle != nullptr);
        testPool->push_back(simEle);
    }




    void startAutoSimTest(PARAM& params){

        std::cout << TC_BLUE << "[kathryn auto test] start auto test\n" << TC_DEF;

        if (testPool == nullptr){
            std::cout << "[kathryn auto test] " << "auto sim has nothing to simulate.\n";
        }

        std::sort(testPool->begin(), testPool->end(), arrangeTestCmp);

        assert(params.find("prefix") != params.end());
        std::string prefixPath = params["prefix"];

        TestReport::instance().clear();

        int testCase = 1;
        for (auto sif: *testPool){
            std::cout << TC_BLUE << "[kathryn auto test] " << "start sim testcase "<< testCase << " id: " << sif->getSimId()<< TC_DEF <<"\n";
            TestReport::instance().beginCase("t" + std::to_string(sif->getSimId()));
            try{
                sif->start(prefixPath, getSPBM(params));
            }catch (const std::exception& e){
                TestReport::instance().markError(e.what());
                std::cout << TC_RED << "[kathryn auto test] testcase " << sif->getSimId()
                          << " threw: " << e.what() << TC_DEF << "\n";
            }
            resetKathryn();
            std::cout << TC_BLUE << "[kathryn auto test] " << "finnish sim testcase "<< testCase << " id: " << sif->getSimId()  << TC_DEF <<"\n";
            testCase++;
            std::cout << TC_BLUE << "----------------------" << TC_DEF << std::endl;
            ////break;
        }

        TestReport::instance().printSummary();

    }









}