//
// Created by tanawin on 4/2/2567.
//


#include "simAutoInterface.h"
#include "simMng.h"

namespace kathryn{


    SimAutoInterface::SimAutoInterface(int simId,
                                       CYCLE limitCycle,
                                       std::string vcdFilePath) :
    SimInterface(limitCycle,std::move(vcdFilePath)),
    _simId(simId){

        /////addSimTestToPool(this);

    }

    void SimAutoInterface::testAndPrint(std::string testName, ValRep &simValLhs, ValRep& testValRhs) {

        if ((simValLhs == testValRhs).getLogicalValue()){
            std::cout << TC_GREEN << testName << " pass " << TC_DEF << std::endl;
        }else{
            std::cout << TC_RED << testName << " fail expect: "
                      << testValRhs.getBiStr() << "  got : "
                      << simValLhs.getBiStr() << TC_DEF << std::endl;
        }
    }


}