//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_MODULE_H
#define KATHRYN_MODULE_H

#include "functional"
#include "vector"
#include "cassert"


#include "gen/proxy_hw_comp/module/module_gen.h"
#include "model/hw_component/global_component/global_component.h"


#include "model/hw_component/abstract/identifiable.h"
#include "model/hw_component/register/register.h"
#include "model/hw_component/wire/wire.h"
#include "model/hw_component/expression/expression.h"
#include "model/hw_component/value/value.h"
#include "model/hw_component/value/pm_val.h"
#include "model/hw_component/mem_block/mem_block.h"
#include "model/hw_component/expression/nest.h"
#include "model/hw_component/box/box.h"
#include "model/interface/base/interface.h"

#include "model/flow_block/abstract/flow_block__base.h"
#include "model/flow_block/abstract/sp_reg/wait_reg.h"
#include "model/flow_block/abstract/nodes/start_node.h"

#include "model/debugger/model_debugger.h"

#include "sim/model_sim_engine/hw_component/module/module_sim.h"
#include "util/logger/logger.h"



namespace kathryn{

    enum MODEL_STAGE{
        MODEL_UNINIT,
        MODEL_GLOB_INITED,
        MODEL_FLOW_INITED,
    };



    class ModuleSimEngine;

    class Module : public Identifiable,
                   public HwCompControllerItf,
                   public ModelDebuggable,
                   public ModuleSimEngineInterface,
                   public ModuleGenInterface
                   {

    private:
        MODEL_STAGE              _mdStage = MODEL_UNINIT;
        bool                     _isTopModule = false;
        /**all slave object that belong to this elements*/
        /** register that user to represent state*/
        std::vector<Reg*>           _spRegs[SP_CNT_REG]; ////// state/ cond/cycle wait use same ctrlflow_regbase class
        std::vector<FlowBlockBase*> _flowBlockBases; //// it contain only head of flowblock in module

        /** user component*/
        std::vector<Reg*>        _userRegs;
        std::vector<Wire*>       _userWires;
        std::vector<expression*> _userExpressions;
        std::vector<Val*>        _userVals;
        std::vector<PmVal*>      _userPmVals;
        std::vector<MemBlock*>   _userMemBlks;
        std::vector<nest*>       _userNests;
        std::vector<Module*>     _userSubModules;
        std::vector<Box*>        _userBoxs; //// it contain only head of box in module
        std::vector<ModelInterface*> _userItfs;

        ModuleSimEngine*         _moduleSimEngine = nullptr;
        ModuleGen*               _moduleGenEngine = nullptr;


        /** when hardware components require data from outside class
         * the system must handle wire routing while synthesis
         * */
    protected:
        /** communicate to controller*/
        void com_init() override;

    public:
        explicit Module(bool init_comp = true);
        ~Module() override;

        void com_final() override;

        template<typename T>
        void delete_sub_element(std::vector<T*>& sub_ele_vec){
            for (auto ele: sub_ele_vec){
                delete ele;
            }
        }
        /** logic comp*/
        /**implicit element that is built from design flow*/
        void add_sp_reg          (Reg* reg, SP_REG_TYPE sp_reg_type);
        void add_flow_block      (FlowBlockBase* fb);

        /**explicit element that is buillt from user declaration*/
        void add_user_reg        (Reg*            reg);
        void add_user_wires      (Wire*           wire);
        void add_user_expression (expression*     expr);
        void add_user_val        (Val*            val);
        void add_user_pm_val      (PmVal*          pm_val); //// parameter value
        void add_user_mem_blk     (MemBlock*       mem_block);
        void add_user_nest       (nest*           nst);
        void add_user_sub_module  (Module*         smd);
        void add_user_box        (Box*            box);
        void add_user_itf        (ModelInterface* itf);


        /**implicit element that is built from design flow*/
        auto& get_sp_regs(SP_REG_TYPE sp_reg_type){
            assert(sp_reg_type < SP_CNT_REG);
            return _spRegs[sp_reg_type];
        }
        auto& get_flow_blocks(){return _flowBlockBases;}
       /**explicit element that is buillt from user declaration*/
        auto& get_user_regs(){return _userRegs; } /** the return contain only master flowblock*/
        auto& get_user_wires(){return _userWires; }
        std::vector<Wire*> get_user_wires_by_marker(WIRE_MARKER_TYPE wmt);
        auto& get_user_expressions(){return _userExpressions; }
        auto& get_user_vals(){return _userVals; }
        auto& get_user_pm_vals(){return _userPmVals;}
        auto& get_user_mem_blks(){return _userMemBlks;}
        auto& get_user_nests(){return _userNests;}
        auto& get_user_sub_modules(){return _userSubModules;}
        auto& get_boxs(){return _userBoxs;}
        auto& get_itfs(){return _userItfs;}

        /** is Top Module*/
        bool is_top_module();
        void set_top_module();


        /** Functions which allow user to custom  their module design flow*/
        MODEL_STAGE  get_stage(){return _mdStage;}
        void         set_stage(MODEL_STAGE md_stage){_mdStage = md_stage;}
        void         build_all();
        virtual void flow(){}; //// user must inherit this function to build thier flow
        virtual void build_flow();
        /** model debug*/
        [[maybe_unused]]
        std::string get_md_describe() override;
        void        add_md_log(MdLogVal* md_log_val) override;
        std::string get_md_ident_val() override{return get_ident_debug_value();};

        ModuleSimEngine* get_sim_engine_ptr() override{
            return _moduleSimEngine;
        }

        void create_module_gen() override;

        ModuleGen*get_module_gen_ptr() override{
            return _moduleGenEngine;
        }



    };



}

#endif //KATHRYN_MODULE_H
