//
// Created by tanawin on 4/11/2024.
//

#include "proxy_build_mode.h"

#include "util/term_color/term_color.h"


namespace kathryn{

    SimProxyBuildMode get_spbm(const PARAM& param){

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
