//
// Created by tanawin on 18/2/2567.
//

#include "mem_block.h"

#include "gen/proxy_hw_comp/mem_block/mem_gen.h"
#include "model/controller/controller.h"
#include "sim/model_sim_engine/hw_component/mem_blk/mem_sim.h"

namespace kathryn{


    MemBlock::MemBlock(ull depth, int width):
            Identifiable(TYPE_MEM_BLOCK),
            HwCompControllerItf(false),
            ModelDebuggable(),
            DEPTH_SIZE(depth),
            WIDTH_SIZE(width),
            mem_sim_engine(new MemSimEngine(this))
    {
        assert(width > 0);
        com_init();
    }

    MemBlock::~MemBlock(){
        for (auto mba: mem_block_agents){
            delete mba;
        }
    }

    void MemBlock::com_init(){
        ctrl->on_memBlk_init(this);
    }

    void MemBlock::init_env_for_mem_holder(MemBlockEleHolder* mem_ele_holder){
        assert(mem_ele_holder != nullptr);
        assert(get_parent_ptr() != nullptr);
        mem_ele_holder->set_parent(get_parent_ptr());
        mem_ele_holder->build_inherit_name();
        mem_block_agents.push_back(mem_ele_holder);
    }


    MemBlockEleHolder& MemBlock::operator[](const Operable& indexer) {
        auto* mem_block_ele_holder = new MemBlockEleHolder(this, &indexer);
        init_env_for_mem_holder(mem_block_ele_holder);
        return *mem_block_ele_holder;
    }

    MemBlockEleHolder& MemBlock::operator[](const int idx) {
        auto* mem_block_ele_holder = new MemBlockEleHolder(this, idx);
        init_env_for_mem_holder(mem_block_ele_holder);
        return *mem_block_ele_holder;
    }

    ValRepBase MemBlock::at(ull idx){
        mf_assert(get_assign_mode() == AM_SIM, "can't retrive memblock while"
                                            " at " +std::to_string(idx) + " loading");
        mf_assert(idx < get_depth_size(), "can't retrive memblock while"
                                            " at " +std::to_string(idx) + " exceed index value");
        return (get_sim_engine_ptr()->get_proxy_rep())[idx];
    }

    void MemBlock::create_logic_gen(){
        _genEngine = new MemGen(
            _parent->get_module_gen_ptr(),
            this
        );
    }

}
