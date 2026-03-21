//
// Created by tanawin on 23/6/2024.
//

#include "mem_gen.h"
#include "mem_agent_gen.h"
#include "gen/proxy_hw_comp/module/module_gen.h"
#include "model/hw_component/mem_block/mem_block_agent.h"

namespace kathryn{

    MemEleholderGen::MemEleholderGen(
        ModuleGen*  md_gen_master,
        MemBlockEleHolder* mh_master
    ): AssignGenBase(
        md_gen_master,
        (Assignable*) mh_master,
        (Identifiable*) mh_master
    ),
    _master(mh_master){}


    void MemEleholderGen::route_dep(){
        //////////// route from indexer
        routed_indexer =
            _mdGenMaster->route_src_opr_to_this_module(_master->get_indexer_ptr());
        //////////// route all from assignable
        AssignGenBase::route_dep();
    }

    std::string MemEleholderGen::dec_variable(){

        if (_master->is_write_mode()){
            return "";
        }

        Slice sl = _master->get_operable_slice();
        ///////// read mode
        return "wire [" + std::to_string(sl.stop-1) +
            ": 0] " + get_opr() + ";";

    }

    std::string MemEleholderGen::dec_op(){

        std::string   master_mem_blk_opr = _master->get_master_mem_blk_ptr()
                                        ->get_logic_gen_ptr()->get_opr();
        std::string   indexer_opr      = get_opr_str_from_opr(routed_indexer);

        if (_master->is_write_mode()){
            return assign_op_base();
        }
        //////////// read_mode
        return "assign " + get_opr() + " = " +
            master_mem_blk_opr + "[" + indexer_opr + "];";

    }

    std::string MemEleholderGen::assignment_line(
        Slice des_slice,
        Operable* src_update_value,
        bool is_delayed_asm
    ){
        assert(is_delayed_asm);
        /////////// it only used in write
        assert(des_slice == _master->get_operable_slice());
        assert(src_update_value != nullptr);

        std::string   master_mem_blk_opr = _master->get_master_mem_blk_ptr()
                                        ->get_logic_gen_ptr()->get_opr();
        std::string   indexer_opr      = get_opr_str_from_opr(routed_indexer);

        return master_mem_blk_opr + "[" + indexer_opr + "] <= " +
            get_opr_str_from_opr(src_update_value) + ";";
    }

}