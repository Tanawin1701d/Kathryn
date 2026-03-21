//
// Created by tanawin on 20/6/2024.
//

#include "mem_gen.h"
#include "model/hw_component/mem_block/mem_block.h"

namespace kathryn{

    MemGen::MemGen(ModuleGen* md_gen_master,
                        MemBlock* mem_block_master):
    LogicGenBase(
        md_gen_master,
        nullptr,
        (Identifiable*) mem_block_master
    ),
    _master(mem_block_master){}


    std::string MemGen::dec_variable(){
        int mem_width = _master->get_width_size();
        int mem_depth = _master->get_depth_size();
        return "reg [" + std::to_string(mem_width-1) +
               ": 0] " + LogicGenBase::get_opr() + " [" +
               std::to_string(mem_depth-1) + ": 0];";
    }

    std::string MemGen::get_opr(Slice sl){
        assert( (sl.start == 0) &&
                (sl.get_size() == _master->get_depth_size()));

        return LogicGenBase::get_opr();
    }



}