//
// Created by tanawin on 26/6/2024.
//

#include "genMng.h"

#include "util/termColor/termColor.h"
#include "params/genParam.h"


namespace kathryn{

    std::vector<GenEle*> genTestCase;


    void addGenEle(GenEle* genEle){
        assert(genEle != nullptr);
        genTestCase.push_back(genEle);
    }

    void startGenEle(PARAM& param){
        std::sort(genTestCase.begin(), genTestCase.end());
        std::string originDesPrefix = param[GEN_PATH_PREFIX];

        for (GenEle* genEle: genTestCase){
            std::cout << TC_BLUE << "gen test case " << genEle->getId() << TC_DEF << std::endl;
            param[GEN_PATH_PREFIX] = originDesPrefix + std::to_string(genEle->getId()) + ".v";
            genEle->startGen(param);
            std::cout << TC_BLUE << "finish gen test case" << genEle->getId() << TC_DEF << std::endl;
        }
    }




}
