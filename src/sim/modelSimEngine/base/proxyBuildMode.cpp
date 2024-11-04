//
// Created by tanawin on 4/11/2024.
//

#include "proxyBuildMode.h"

#include "util/termColor/termColor.h"


namespace kathryn{

    SimProxyBuildMode getSPBM(const PARAM& param){

        auto iter = param.find(param_spb_key);

        if (iter == param.end()){
            std::cout << TC_RED << "error sim proxy build mode is not defined" << TC_DEF << std::endl;
            assert(false);
        }

        std::string value = iter->second;

        SimProxyBuildMode mode = SimProxyBuildMode::SPB_NON;
        if(value.find(param_spb_g) != std::string::npos){
            mode = mode | SimProxyBuildMode::SPB_GEN;
        }
        if(value.find(param_spb_c) != std::string::npos){
            mode = mode | SimProxyBuildMode::SPB_COMPILE;
        }
        if(value.find(param_spb_r) != std::string::npos){
            mode = mode | SimProxyBuildMode::SPB_RUN;
        }
        return mode;

    }

}
