//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_MODEL_CONTROLLER_H
#define KATHRYN_MODEL_CONTROLLER_H

#include "stack"
#include "memory"

#include "abstract/main_controlable.h"

#include "model/hw_component/module/module.h"
#include "model/flow_block/abstract/nodes/asm_node.h"
#include "model/flow_block/abstract/flow_block__base.h"

/** flow block include */

#include "model/flow_block/seq/seq.h"
#include "model/flow_block/par/par.h"
#include "model/flow_block/loop/while_base.h"
#include "model/flow_block/loop/do_while_base.h"
#include "model/flow_block/loop/cbreak.h"
#include "model/flow_block/loop/loop.h"
#include "model/flow_block/cond/zif.h"
#include "model/flow_block/cond/zelif.h"
#include "model/flow_block/cond/if.h"
#include "model/flow_block/cond/elif.h"
#include "model/flow_block/pick/pick.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/sp_reg/sync_reg.h"
#include "model/flow_block/time/wait.h"
#include "model/hw_component/mem_block/mem_block.h"
#include "model/flow_block/pipeline/pipe.h"
#include "model/flow_block/pipeline/sync_meta.h"
#include "model/flow_block/pipeline/zync.h"
#include "model/flow_block/pseudo/pseudo.h"
#include "model/flow_block/state/ztate.h"
#include "model/flow_block/state/zcase.h"
#include "model/flow_block/pipe_stream/pipe_stream.h"


/** data Structure*/

#include "model/hw_collection/data_structure/slot/reg_slot.h"
#include "model/hw_collection/data_structure/slot/wire_slot.h"
#include "model/hw_collection/data_structure/table/table.h"
#include "model/hw_collection/data_structure/mux/mux.h"


/** clk mode*/
#include "model/controller/clock_mode.h"


#include "model/interface/single_hand_shake/shs.h"


#include "util/type/type_conv.h"
#include "util/str/str_util.h"



namespace kathryn {

    enum MODULE_BUILDING_STATE{
        MODULE_INIT, /** module element is declared but not yet init design flow only for element initialization*/
        MODULE_END_GLOB_DECLARE, /** module end init element*/
        MODULE_INIT_DESIGN_FLOW, /** module start init design flow*/
        MODULE_END, /** module finalize*/

        MODULE_INIT_AUX,
        MODULE_FINAL_AUX

    };

    struct Module_Stack_Element{
        Module* md;
        MODULE_BUILDING_STATE state;
    };


    class MainControlable;

    class ModelController : public MainControlable {

    private:
        bool hw_comp_alloc_lock = true; /** this is used to indicate whether make<> is used or not only make<> can unlock*/
        /** building stack*/
        ////// module stack
        std::stack<Module_Stack_Element>  module_stack;
        ////// box stack to determine which hw element belong to stack
        std::stack<Box*> box_stack;
        ////// flow describe stack
        std::stack<FlowBlockBase*> flow_block_stacks[FLOW_ST_CNT];
        /////// pattern flow block is subset of flow_block_stack
        Module* global_module_ptr = nullptr;

    protected:
        /** get module that response we now consider*/
        Module* get_top_module_ptr();
        Module_Stack_Element& get_target_module_ele();

       void           pop_flow_block(FlowBlockBase* fb);
       void           push_flow_block(FlowBlockBase* fb);
       void           detach_top_flow_block();
    public:
        FlowBlockBase*get_top_flow_block_base_ptr();
        FlowBlockBase*get_top_flow_block_base_ptr(FLOW_STACK_TYPE stack_enum);


    public:

        /**
         *
         * Hardware component handling fucntion
         *
         * */
        explicit ModelController();
        void start() override;
        void reset() override;
        void clean() override;
        Module*get_global_module_ptr();
        /** state register handling*/
        void on_sp_reg_init(CtrlFlowRegBase* ptr, SP_REG_TYPE reg_type);
        /** register handling*/
        void on_reg_init(Reg* ptr);
        void on_reg_update(AsmNode* asm_node, Reg* src_reg);
        /** wire handling*/
        void on_wire_init(Wire* ptr);
        void on_wire_update(AsmNode* asm_node, Wire* src_wire);
        /** expr_metas handling*/
        void on_expression_init(expression* ptr);
        /** on mem_block and its agent is updated*/
        void on_memBlk_init(MemBlock* ptr);
        void on_memBlkEleHolder_update(AsmNode* asm_node,MemBlockEleHolder* src_holder);
        /** on nest init*/
        void on_nest_init(nest* ptr);
        void on_nest_update(AsmNode* asm_node, nest* src_nest);
        /** value and pm value handling*/
        void on_value_init(Val* ptr);
        void on_pmValue_init(PmVal* ptr);
        /** box handling*/
        void on_box_init(Box* ptr);
        void on_box_end_init(Box* ptr);
        void on_box_update(AsmNode* asm_node, Box* box);
        void on_box_tryAddToBox(Operable* opr1, Assignable* asb);
        /** interface handling*/
        void on_itf_init(ModelInterface* ptr);

        //void on_chk_and_lock_belongBlk(Assignable* asb, Operable* opr);
        //void on_chk_and_release_blk(Assignable* asb);

        /**
         *
         * module handling
         *
         * */

        ////// for global module
        void on_globalModule_init_component();
        void on_globalModule_init_designFlow();
        void on_globalModule_init_auxilaryComponent(); ///// for init some readonly logic for simulation trigger
        void on_globalModule_final_auxilaryComponent();
        ////// for nomal module
        void on_module_init_components(Module* ptr);
        void on_module_end_init_components(Module* ptr);
        void on_module_init_designFlow(Module* ptr); /** todo make design flow implement correctly*/
        void on_module_final(Module* ptr);



        /**
         *
         * control flow block handler
         *
         * */

        void assign_flow_block_parent(FlowBlockBase* fb);
        bool is_all_flow_stack_empty();
        bool is_flow_stack_empty(FLOW_STACK_TYPE flow_stack_type);
        bool is_top_fb_belong_to_top_module();
        void try_purify_flow_stack();
        void on_attach_flowBlock(FlowBlockBase* fb);
        void on_detach_flowBlock(FlowBlockBase* fb);
        void on_attachAndDetach_intrSignal(INT_TYPE int_type, Operable* sig);
        //Operable& on_get_check_next_pipblk_ready_signal();
        FLOW_BLOCK_TYPE get_top_pattern_flow_block_type();

        bool is_top_of_stack_belong_to_the_same_module(FLOW_STACK_TYPE a, FLOW_STACK_TYPE b);

        /** lock allocation*/
        void lock_allocation() {hw_comp_alloc_lock = true;};
        void unlock_allocation(){hw_comp_alloc_lock = false;};
        [[nodiscard]]
        bool is_allocation_lock() const{return hw_comp_alloc_lock;}

        /** for debugging and model checking purpose*/
        std::string get_cur_model_stack();

        /**
         *
         * pipeline controller
         *
         * */

    };

    /** this is entrace for every device to com_init with controller*/
    ModelController* get_controller_ptr();
    Module*          get_global_module_ptr();
    void             free_controller_ptr();

}

#endif //KATHRYN_ModelCONTROLLER_H
