//
// Created by tanawin on 20/6/2024.
//

#include "module_gen.h"

#include "model/hw_component/module/module.h"
#include "model/hw_component/abstract/glob_pool.h"
#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"

namespace kathryn{


    ModuleGen::ModuleGen(Module* master):
    _master(master){
        assert(_master != nullptr);
    }

    void ModuleGen::start_init_ele(){

        if (_master->get_parent_ptr() == nullptr){
            /// current module is top module
            depth_from_global_module = 0;
        }else{
            depth_from_global_module = _master->get_parent_ptr()
                                    ->get_module_gen_ptr()->get_dept() + 1;
        }

        //////// init the sub module elements first
        for (Module* sub_module: _master->get_user_sub_modules()){
            sub_module->create_module_gen(); //// create submodule gen first
            sub_module->get_module_gen_ptr()->start_init_ele(); //// init sub module element
            _subModulePool.push_back(sub_module->get_module_gen_ptr()); //// add to this module pool
        }

        //////// init all logic element wo test
        /////////////////// logic element should not be recursively called, it should be atomic
        for (int sp_idx = 0; sp_idx < SP_CNT_REG; sp_idx++){
            create_and_recruit_logic_gen_base(
                _regPool,
                _master->get_sp_regs((SP_REG_TYPE)sp_idx));
        }
        create_and_recruit_logic_gen_base(_regPool, _master->get_user_regs());
        ////// for wire we must seperate marker for input and output of module mark
        for (Wire* wire: _master->get_user_wires()){
            wire->create_logic_gen();
            switch (wire->get_marker()){
                case WMT_INPUT_MD:{
                    _wirePoolWithInputMarker.push_back(wire->get_logic_gen_ptr());
                    break;
                }
                case WMT_OUTPUT_MD:{
                    _wirePoolWithOutputMarker.push_back(wire->get_logic_gen_ptr());
                    break;
                }
                default:{
                    _wirePool.push_back(wire->get_logic_gen_ptr());
                    break;
                }
            }
        }
        create_and_recruit_logic_gen_base(_exprPool    ,_master->get_user_expressions());
        create_and_recruit_logic_gen_base(_nestPool    ,_master->get_user_nests());
        create_and_recruit_logic_gen_base(_valPool     ,_master->get_user_vals());
        create_and_recruit_logic_gen_base(_pmValPool   , _master->get_user_pm_vals());
        create_and_recruit_logic_gen_base(_memBlockPool, _master->get_user_mem_blks());
        for (MemBlock* mem_block: _master->get_user_mem_blks()){
            create_and_recruit_logic_gen_base(_memBlockElePool, mem_block->get_mem_block_agents());
        }

    }
}
