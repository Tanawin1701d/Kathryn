//
// Created by tanawin on 31/5/2024.
//

#ifndef MODEL_SIMINTF_MEMSIMENGINE_H
#define MODEL_SIMINTF_MEMSIMENGINE_H


#include "sim/model_sim_engine/base/model_proxy.h"
#include "sim/model_sim_engine/base/proxy_event_base.h"


namespace kathryn{

    class MemBlock;

    class MemSimEngine: public ModelProxyBuild, public ModelProxyRetrieve{
    private:
        MemBlock* _master = nullptr;
    public:
        explicit MemSimEngine(MemBlock* master);

        ///////// create c++
        void                     proxy_build_init()               override{};
        ValR                     get_val_rep()                    override;
        std::vector<std::string> get_regis_var_name()              override;
        ull                      get_var_id()                     override;
        SIM_VALREP_TYPE_ALL      getValR_Type()                 override;
        void                     mark_sv(const std::string& str) override;
        
        void create_global_variable(CbBaseCxx& cb) override;
        void create_local_variable (CbBaseCxx& cb) override{}
        void create_op            (CbBaseCxx& cb) override{}
        void create_op_end_cycle    (CbBaseCxx& cb) override{}
        void create_op_end_cycle2   (CbBaseCxx& cb) override{}
        void create_user_mark_value (CbBaseCxx& cb) override;

        CLOCK_MODE get_clock_mode() override {return CM_CLK_UNUSED;}

        bool        is_user_declare()        override{return false;}
        void        proxy_ret_init(ProxySimEventBase* model_sim_event)override;
        ValRepBase& get_proxy_rep()          override;

    };

    class MemSimEngineInterface{
    public:
        virtual void mark_sv(const std::string& key) = 0;
        virtual ~MemSimEngineInterface() = default;
        virtual MemSimEngine* get_sim_engine_ptr() = 0;
    };



}

#endif //MODEL_SIMINTF_MEMSIMENGINE_H
