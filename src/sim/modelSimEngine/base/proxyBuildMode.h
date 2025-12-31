//
// Created by tanawin on 4/11/2024.
//

#ifndef src_sim_modelSimEngine_base_PROXYBUILDMODE_H
#define src_sim_modelSimEngine_base_PROXYBUILDMODE_H
#include <cstdint>
#include "frontEnd/cmd/paramReader.h"

namespace kathryn{

    //////// run mode

    enum class SimProxyBuildMode: uint8_t{
        SPB_NON         = 0,
        SPB_GEN         = 1 << 0,
        SPB_COMPILE     = 1 << 1,
        SPB_RUN         = 1 << 2
    };

    inline SimProxyBuildMode
    operator | (SimProxyBuildMode a, SimProxyBuildMode b){
        return static_cast<SimProxyBuildMode>(
            static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
        );
    }

    inline SimProxyBuildMode
    operator & (SimProxyBuildMode a, SimProxyBuildMode b){
        return static_cast<SimProxyBuildMode>(
            static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
        );
    }
    ////// has config b
    inline bool hasConfig(SimProxyBuildMode a, SimProxyBuildMode b){
        return (a & b) == b;
    }


    constexpr char param_spb_key[] = "buildSimMode";
    constexpr char param_spb_g     = 'g';
    constexpr char param_spb_c     = 'c';
    constexpr char param_spb_r     = 'r';

    SimProxyBuildMode getSPBM(const PARAM& param);

}

#endif //src_sim_modelSimEngine_base_PROXYBUILDMODE_H
