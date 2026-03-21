//
// Created by tanawin on 20/6/2024.
//

#ifndef MEMGEN_H
#define MEMGEN_H
#include "model/hw_component/mem_block/mem_block.h"
#include "gen/proxy_hw_comp/abstract/logic_gen_base.h"


namespace kathryn{



    class MemGen: public LogicGenBase{

        MemBlock* _master = nullptr;

    public:
        explicit MemGen(ModuleGen* md_gen_master,
                        MemBlock* mem_block_master);

        /////// route dep
        void route_dep() override{};

        std::string dec_io      () override {return "";}
        std::string dec_variable() override;
        std::string dec_op      () override {return "";}

        std::string get_opr(Slice sl) override;

    };
    //



}

#endif //MEMGEN_H
