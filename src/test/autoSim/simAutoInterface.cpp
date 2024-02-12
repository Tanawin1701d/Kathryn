//
// Created by tanawin on 4/2/2567.
//


#include "simAutoInterface.h"

#include <utility>
#include "simMng.h"

namespace kathryn{


    SimAutoInterface::SimAutoInterface(int simId,
                                       CYCLE limitCycle,
                                       std::string vcdFilePath,
                                       std::string profileFilePath
                                       ) :
    SimInterface(limitCycle,
                 std::move(vcdFilePath),
                 std::move(profileFilePath)),
    _simId(simId){
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