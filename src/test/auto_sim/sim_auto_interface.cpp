//
// Created by tanawin on 4/2/2567.
//


#include "sim_auto_interface.h"

#include "utility"
#include "sim_mng.h"

namespace kathryn{


    SimAutoInterface::SimAutoInterface(int sim_id,
                                       CYCLE limit_cycle,
                                       std::string vcd_file_path,
                                       std::string profile_file_path,
                                       SimProxyBuildMode sim_proxy_build_mode,
                                       bool req_inline,
                                       int  op_level,
                                       const std::string& gen_pref
                                       ) :
    SimInterface(limit_cycle,
                 std::move(vcd_file_path),
                 std::move(profile_file_path),
                 "simple_gen_file_" + gen_pref + "_" + std::to_string(sim_id),
                 sim_proxy_build_mode,
                 false,
                 req_inline,
                 op_level),
    _simId(sim_id){
    }




    }
