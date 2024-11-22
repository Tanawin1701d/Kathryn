//
// Created by tanawin on 22/11/2024.
//

#ifndef KATHRYN_SRC_MODEL_HWCOMPONENT_MODULE_MULTITHREADMODULE_MTMODULE_H
#define KATHRYN_SRC_MODEL_HWCOMPONENT_MODULE_MULTITHREADMODULE_MTMODULE_H

#include"model/hwComponent/module/module.h"
#include"mtAgent.h"

namespace kathryn{




    /////////// _ME suffix standfor multithreading element

class MtModule: public Module{
protected:

    //////// controller
    MtCtrlAgent mtAgent;

    //////// scratchPad
    Reg*                   _IN_SCP = nullptr;
    std::vector<Reg*>      _IN_SCP_memBlockIndexers;
    std::vector<MemBlock*> _userMemBlock_IN_SCP;


    std::vector<Reg*>      _spRegs_OUT_SCP[SP_CNT_REG]; //// the register that is used to store the  prefetched data from dram for special register
    std::vector<Reg*>      _userRegs_OUT_SCP;
    std::vector<Reg*>      _OUT_SCP_memBlockIndexers;
    std::vector<MemBlock*> _userMemBlock_OUT_SCP;

    //////// build flow for CONTROLLER AGENT, SC RATCH PAD, MEM CONTROLLER, AND ACTUAL MODEL
    void auxFlow() override;
    void augmentModule() override;

    //////// augment section
    void augRegRcvScp(std::vector<Reg*>& regs);
    void augRegRcvScp(Reg* reg);

    void augMemsRcvScp(std::vector<MemBlock*>& memBlks);
    void augMemsRcvScp(MemBlock* memBlk);


    //////// getter setter

    MtCtrlAgent& getMtAgent(){return mtAgent;};





};


}

#endif //KATHRYN_SRC_MODEL_HWCOMPONENT_MODULE_MULTITHREADMODULE_MTMODULE_H
