//
// Created by tanawin on 29/3/2567.
//



#ifndef KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H
#define KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H

#include "sim/model_sim_engine/base/model_proxy.h"
#include "sim/sim_res_writer/sim_res_writer.h"

namespace kathryn{

    class FlowBlockBase;
    struct Node;


    /**
     *
     * node is not include anymore
     */

    constexpr int SUB_FLOWBLOCK_GEN_OP_SPACE = 4;

    class FlowBlockBase;
    class FlowBaseSimEngine: public ModelProxyBuild,
                             public ModelProxyRetrieve{
    protected:
        FlowBlockBase* _flowBlockBase  = nullptr;
        ValRepBase     _proxyRepCurBit;

    public:
        FlowBaseSimEngine(FlowBlockBase* flow_block_base);
        ~FlowBaseSimEngine() = default;
        void                     proxy_build_init() override{};
        ValR                     get_val_rep()     override;
        std::vector<std::string> get_regis_var_name() override;
        ValR                     get_var_name_cur_status();
        ull                      get_var_id()       override;
        SIM_VALREP_TYPE_ALL      getValR_Type()   override;
        CLOCK_MODE               get_clock_mode()   override;
        void                     get_recur_var_name(std::vector<std::string>& result);
        void                     get_recur_var_name_cur_stsatus(std::vector<std::string>& result);

        void create_global_variable (CbBaseCxx& cb) override;
        void create_local_variable  (CbBaseCxx& cb) override{}
        void create_op             (CbBaseCxx& cb) override; ///// we use op to do all operation
        void create_op_end_cycle     (CbBaseCxx& cb) override{}
        void create_op_end_cycle2    (CbBaseCxx& cb) override{}
        void create_user_mark_value  (CbBaseCxx& cb) override{}
        bool        is_user_declare()          override{return false;}

        ////////
        ///
        void        proxy_ret_init(ProxySimEventBase* model_sim_event)     override;
        ValRepBase& get_proxy_rep()      override;
        bool        is_block_running();


    };

    class FlowSimEngineInterface{
    public:
        virtual FlowBaseSimEngine* get_sim_engine_ptr() = 0;
    };




}

#endif //KATHRYN_SIMITF_FLOWBLOCK_FLOWBLOCKSIMENGINE_H
