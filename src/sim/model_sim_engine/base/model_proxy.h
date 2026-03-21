//
// Created by tanawin on 31/5/2024.
//

#ifndef MODELPROXY_H
#define MODELPROXY_H

#include "cassert"
#include "utility"


#include "model/hw_component/abstract/operable.h"
#include "proxy_event_base.h"
#include "sim/sim_res_writer/sim_res_writer.h"
#include "model/abstract/ident_base/ident_base.h"
#include "sim/logic_rep/val_rep.h"
#include "util/file_writer/code_writer/cpp_writer.h"
#include "sim_val_type.h"
#include "model/controller/clock_mode.h"


namespace kathryn{

    constexpr int MAX_VAL_REP_SIZE = 64;


    class ModelProxyBuild{
    protected:
        std::vector<ModelProxyBuild*> dep;
        std::string TEMP_VAR_SUFFIX = "_TEMP";

        ///// marksv is used to explicit var name to make designer modify the
        ///// model file like verilator
        std::string markSV_key          = "undefined";
        bool        is_mark_sv        = false;
    public:
        virtual             ~ModelProxyBuild()= default;
        virtual void        proxy_build_init()   = 0;
        //// pre initialize section fill dependency
        virtual ValR                     get_val_rep      () = 0; //// prefix is not include
        virtual std::vector<std::string> get_regis_var_name() = 0;
        virtual ull                      get_var_id       () = 0; //// it is global id
        std::vector<ModelProxyBuild*>&   get_dep         () {return dep;}
        virtual SIM_VALREP_TYPE_ALL      getValR_Type   () = 0;
        void                             markSV_base    (const std::string& str){is_mark_sv = true; markSV_key = str;}
        virtual void                     mark_sv         (const std::string& str){assert(false);}

        virtual CLOCK_MODE               get_clock_mode   () = 0;

        //// c++ create section
        virtual void create_global_variable (CbBaseCxx& cb) = 0;
        virtual void create_local_variable  (CbBaseCxx& cb) = 0;
        virtual void create_op             (CbBaseCxx& cb) = 0; //// compute the data but
        virtual void create_op_end_cycle     (CbBaseCxx& cb) = 0; //// have more priority
        virtual void create_op_end_cycle2    (CbBaseCxx& cb) = 0; //// have less priority
        virtual void create_user_mark_value  (CbBaseCxx& cb) = 0; //// create designer defined name visable to manual overide
        virtual bool is_user_declare()        = 0;


    };

    class ModelProxyRetrieve{
    protected:
        ValRepBase proxy_rep;
    public:
        virtual void        proxy_ret_init(ProxySimEventBase* model_sim_event) = 0;
        virtual ValRepBase& get_proxy_rep () = 0;
        virtual      ~ModelProxyRetrieve() = default;
    };

    

}

#endif //MODELPROXY_H
