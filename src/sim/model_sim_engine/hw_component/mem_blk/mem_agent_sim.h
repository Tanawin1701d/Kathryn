//
// Created by tanawin on 18/7/2024.
//

#ifndef MEMAGENTSIM_H
#define MEMAGENTSIM_H

#include "sim/model_sim_engine/hw_component/abstract/logic_sim_engine.h"

namespace kathryn{


    class MemBlockEleHolder;
    class MemEleHolderSimEngine: public LogicSimEngine{
        const std::string IS_SET_SUFFIX = "_isSet";
        const std::string INDEXER_SUFFIX = "_indexer";
        MemBlockEleHolder* _master = nullptr;
    public:
        explicit MemEleHolderSimEngine(MemBlockEleHolder* master);

        void        proxy_build_init()    override;

        void create_global_variable(CbBaseCxx& cb) override;
        void create_local_variable (CbBaseCxx& cb) override;

        void create_op         (CbBaseCxx& cb) override;
        void create_op_end_cycle (CbBaseCxx& cb) override;
        void create_op_end_cycle2(CbBaseCxx& cb) override{}

        ValR get_is_set_var();
        ValR get_indexer_var();

        void create_op_read_mode (CbBaseCxx& cb);
        void create_op_write_mode(CbBaseCxx& cb);

    };

}

#endif //MEMAGENTSIM_H
