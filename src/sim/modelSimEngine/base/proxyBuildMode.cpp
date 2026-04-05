//
// Created by tanawin on 4/11/2024.
//

#include "proxyBuildMode.h"

#include "util/termColor/termColor.h"


namespace kathryn{

    SimProxyBuildMode getSpbm(const PARAM& param){

        auto iter = param.find(paramSpbKey);

        if (iter == param.end()){
            std::cout << TC_RED << "error sim proxy build mode is not defined" << TC_DEF << std::endl;
            assert(false);
        }

        std::string value = iter->second;

        SimProxyBuildMode mode = SimProxyBuildMode::SPB_NON;
        if(value.find(paramSpbG) != std::string::npos){
            mode = mode | SimProxyBuildMode::SPB_GEN;
        }
        if(value.find(paramSpbC) != std::string::npos){
            mode = mode | SimProxyBuildMode::SPB_COMPILE;
        }
        if(value.find(paramSpbR) != std::string::npos){
            mode = mode | SimProxyBuildMode::SPB_RUN;
        }
        return mode;

    }

}
