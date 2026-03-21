//
// Created by tanawin on 21/2/2567.
//

#ifndef KATHRYN_LOGICSIMENGINE_H
#define KATHRYN_LOGICSIMENGINE_H

#include "sim/sim_res_writer/sim_res_writer.h"
#include "sim/model_sim_engine/base/model_proxy.h"
#include "sim/model_sim_engine/base/proxy_event_base.h"
#include "model/hw_component/abstract/assignable.h"
#include "model/hw_component/abstract/slice.h"
#include "util/file_writer/code_writer/cpp_writer.h"



namespace kathryn{

    class LogicSimEngine: public ModelProxyBuild, public ModelProxyRetrieve{

    protected:

        Assignable*   _asb                = nullptr;
        Identifiable* _ident              = nullptr;
        Operable*     _opr                = nullptr;
        VCD_SIG_TYPE  _vcdSigType         = VST_DUMMY;
        bool          _setToWrite         = false;
        bool          _isTempReq          = false; ///// request temp request
        bool          _flowPerfBit        = false; ////// it is used to set that this logic is used to be perf profiler
        ull           _initVal            = 0;
        bool          _reqGlobDec         = false;

    public:
        //////////////////////// gen main operation


        void create_op_with_sole_condition        (CbBaseCxx& cb, const std::string& aux_ass_str = "");
        //void gen_op_with_chain_condition          (CbBaseCxx& cb, const std::string& aux_ass_str = "");
        std::string gen_assign_a_eq_b             (Slice     des_slice, bool is_des_temp,
                                               Operable* src_opr);
        ///////////////////////// sliced opr
        virtual ValR gen_src_opr                ();
        virtual ValR gen_sliced_opr_to           (Slice src_slice, SIM_VALREP_TYPE_ALL des_field);
        virtual ValR gen_sliced_opr_and_shift     (Slice des_slice, Slice src_slice, SIM_VALREP_TYPE_ALL des_field);


        LogicSimEngine(Assignable* asb, Identifiable*   ident, Operable* opr1,
                       VCD_SIG_TYPE sig_type, bool is_temp_req,
                        ull init_val
                       );

        void proxy_build_init() override;

        //std::string              get_var_name()      override;
        ValR                     get_val_rep() override;
        std::vector<std::string> get_regis_var_name() override;
        ValR                     get_temp_val_rep();
        ull                      get_var_id()        override{return _ident->get_global_id();}
        SIM_VALREP_TYPE_ALL      getValR_Type() override;
        CLOCK_MODE               get_clock_mode() override;
        void                     set_vcd_write_status(bool status){ _setToWrite = status;}

        [[nodiscard]]
        VCD_SIG_TYPE             get_sig_type() const {return _vcdSigType;}
        [[nodiscard]]
        Slice                    get_size()    const {return _asb->get_assign_slice();}


        /*** c++ create section**/

        void create_global_variable(CbBaseCxx& cb) override;
        void create_local_variable (CbBaseCxx& cb) override{}
        void create_op            (CbBaseCxx& cb) override;
        void create_op_end_cycle    (CbBaseCxx& cb) override{}
        void create_op_end_cycle2   (CbBaseCxx& cb) override;
        void create_user_mark_value (CbBaseCxx& cb) override; //// create designer defined name visable to manual overide


        bool        is_user_declare()       override{return _ident->is_user_var();}
        bool        is_temp_req()                   {return _isTempReq;}

        void        set_flow_block_iden(bool flow_iden){_flowPerfBit = flow_iden;}
        void        req_glob_dec(){ _reqGlobDec = true;}

        /////// proxy
        ///

        void proxy_ret_init(ProxySimEventBase* model_sim_event)       override;
        ValRepBase& get_proxy_rep() override;
    };

    class LogicSimEngineInterface{
    public:
        //////// it is used to mask the signal which designer can make modify it
        /// in cpp generated simulation model easier
        virtual void mark_sv(const std::string& key) = 0;
        virtual ~LogicSimEngineInterface() = default;
        virtual LogicSimEngine* get_sim_engine_ptr() = 0;
    };

}

#endif //KATHRYN_LOGICSIMENGINE_H
