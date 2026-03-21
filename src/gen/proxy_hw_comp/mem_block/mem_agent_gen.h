//
// Created by tanawin on 23/6/2024.
//

#ifndef MEMAGENTGEN_H
#define MEMAGENTGEN_H

#include "gen/proxy_hw_comp/abstract/assign_gen.h"



namespace kathryn{

    class MemBlockEleHolder;
    class MemEleholderGen: public AssignGenBase{
    protected:
        MemBlockEleHolder* _master     = nullptr;
        Operable*          routed_indexer = nullptr;
    public:
        explicit MemEleholderGen(
            ModuleGen*  md_gen_master,
            MemBlockEleHolder* mh_master
        );

        void route_dep() override;

        std::string dec_io() override {assert(false);}
        std::string dec_variable() override;
        std::string dec_op() override;

        std::string assignment_line(Slice des_slice, Operable* src_update_value, bool is_delayed_asm) override;
    };

}

#endif //MEMAGENTGEN_H
