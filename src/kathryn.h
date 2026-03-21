//
// Created by tanawin on 3/2/2567.
//

#ifndef KATHRYN_KATHRYN_H
#define KATHRYN_KATHRYN_H


#include "model/controller/controller.h"
#include "gen/controller/gen_controller.h"
#include "front_end/cmd/param_reader.h"
#include "sim/interface/sim_interface.h"


#include "util/file_writer/slot_writer/slot_writer.h"
#include "model/hw_component/abstract/glob_pool.h"
#include "util/logger/logger.h"

namespace kathryn{

    void start_model_kathryn();
    void start_gen_kathryn(PARAM& params);
    void reset_kathryn();
    PARAM read_param_kathryn(std::string file_path);

}

#endif //KATHRYN_KATHRYN_H
