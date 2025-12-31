//
// Created by tanawin on 18/7/2024.
//

#include "registerSim.h"
#include "model/hwComponent/register/register.h"


namespace kathryn{

    RegSimEngine::RegSimEngine(Reg* master,
                         VCD_SIG_TYPE sigType):
        LogicSimEngine(master, master, master, sigType, true, 0),
        _master(master){
        assert(master != nullptr);
    }

}