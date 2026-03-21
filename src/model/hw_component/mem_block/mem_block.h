//
// Created by tanawin on 18/2/2567.
//

#ifndef KATHRYN_MEMBLOCK_H
#define KATHRYN_MEMBLOCK_H

#include "map"
#include "sim/model_sim_engine/hw_component/mem_blk/mem_sim.h"
#include "model/hw_component/abstract/identifiable.h"
#include "model/controller/con_interf/controller_itf.h"
#include "model/debugger/model_debugger.h"
#include "model/hw_component/abstract/operable.h"
#include "util/numberic/pmath.h"
#include "gen/proxy_hw_comp/mem_block/mem_gen.h"

#include "mem_block_agent.h"


namespace kathryn{

    class MemBlock: public Identifiable,
                    public HwCompControllerItf,
                    public MemSimEngineInterface,
                    public LogicGenInterface,
                    public ModelDebuggable{
    private:
        const ull DEPTH_SIZE = 0;
        const int WIDTH_SIZE = 0;

        std::vector<MemBlockEleHolder*> mem_block_agents;

        MemSimEngine* mem_sim_engine = nullptr;
        LogicGenBase* _genEngine = nullptr;

    public:
        explicit MemBlock(ull depth, int width);
        ~MemBlock();

        void com_init() override;
        void com_final() override{};

        void init_env_for_mem_holder(MemBlockEleHolder* mem_ele_holder);

        MemBlockEleHolder& operator[] (const Operable& indexer);
        MemBlockEleHolder& operator[] (const int idx);

        int get_width_size() const {assert(WIDTH_SIZE != 0); return WIDTH_SIZE;}
        ull get_depth_size() const {assert(DEPTH_SIZE != 0); return DEPTH_SIZE;}
        auto& get_mem_block_agents(){return mem_block_agents;}

        ValRepBase at(ull idx);

        /** override debugger*/
        std::string get_md_ident_val() override{
            return get_ident_debug_value();
        }

        MemSimEngine* get_sim_engine_ptr()                 override{return mem_sim_engine;}
        void          mark_sv(const std::string& str) override{mem_sim_engine->mark_sv(str);}

        void          create_logic_gen() override;
        LogicGenBase* get_logic_gen_ptr() override{return _genEngine;}

    };

}

#endif //KATHRYN_MEMBLOCK_H
