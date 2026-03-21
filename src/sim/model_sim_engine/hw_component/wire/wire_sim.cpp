//
// Created by tanawin on 18/7/2024.
//

#include "wire_sim.h"

#include "model/hw_component/wire/wire.h"


namespace kathryn{


    /**
         *
         *   wire sim engine
         *
         ***/
    WireSimEngine::WireSimEngine(Wire* master, VCD_SIG_TYPE sig_type):
    LogicSimEngine(
        (Assignable*  ) master,
        (Identifiable*) master,
        (Operable*)     master,
        VST_WIRE,
        false,
        0),
    _master(master){
        ///std::cout << "wire simengine is created" << std::endl;
        //std::cout << master->get_var_name() << std::endl;
        assert(_master != nullptr);
    }



}
