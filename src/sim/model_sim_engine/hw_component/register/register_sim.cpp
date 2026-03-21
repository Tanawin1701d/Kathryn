//
// Created by tanawin on 18/7/2024.
//

#include "register_sim.h"
#include "model/hw_component/register/register.h"


namespace kathryn{

    RegSimEngine::RegSimEngine(Reg* master,
                         VCD_SIG_TYPE sig_type):
        LogicSimEngine(master, master, master, sig_type, true, 0),
        _master(master){
        assert(master != nullptr);
    }

}