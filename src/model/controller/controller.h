//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_MODEL_CONTROLLER_H
#define KATHRYN_MODEL_CONTROLLER_H

#include <stack>
#include <memory>

#include "abstract/mainControlable.h"

#include "model/hwComponent/module/module.h"
#include "model/flowBlock/abstract/nodes/asmNode.h"
#include "model/flowBlock/abstract/flowBlock_Base.h"

/** flow block include */

#include "model/flowBlock/seq/seq.h"
#include "model/flowBlock/par/par.h"
#include "model/flowBlock/loop/whileBase.h"
#include "model/flowBlock/loop/doWhileBase.h"
#include "model/flowBlock/loop/cbreak.h"
#include "model/flowBlock/loop/loop.h"
#include "model/flowBlock/cond/zif.h"
#include "model/flowBlock/cond/zelif.h"
#include "model/flowBlock/cond/if.h"
#include "model/flowBlock/cond/elif.h"
#include "model/flowBlock/pick/pick.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/spReg/syncReg.h"
#include "model/flowBlock/time/wait.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "model/flowBlock/pipeline/pipe.h"
#include "model/flowBlock/pipeline/syncMeta.h"
#include "model/flowBlock/pipeline/zync.h"
#include "model/flowBlock/pseudo/pseudo.h"
#include "model/flowBlock/state/ztate.h"
#include "model/flowBlock/state/zcase.h"
#include "model/flowBlock/pipeStream/pipeStream.h"


/** data Structure*/

#include "model/hwCollection/dataStructure/slot/regSlot.h"
#include "model/hwCollection/dataStructure/slot/wireSlot.h"
#include "model/hwCollection/dataStructure/table/table.h"
#include "model/hwCollection/dataStructure/mux/mux.h"


/** clk mode*/
#include "model/controller/clockMode.h"


#include "model/interface/singleHandShake/shs.h"


#include "util/type/typeConv.h"
#include "util/str/strUtil.h"



namespace kathryn {

    enum MODULE_BUILDING_STATE{
        MODULE_INIT,             /// module element is declared but not yet init design flow only for element initialization
        MODULE_END_GLOB_DECLARE, /// module end init element
        MODULE_INIT_DESIGN_FLOW, /// module start init design flow
        MODULE_END,              /// module finalize

        MODULE_INIT_AUX,         /// module start init auxilary component
        MODULE_FINAL_AUX

    };

    struct Module_Stack_Element{
        Module* md;
        MODULE_BUILDING_STATE state;
    };


    class MainControlable;

    class ModelController : public MainControlable {
    private:
        bool _hw_comp_alloc_lock = true; /** this is used to indicate whether make<> is used or not only make<> can unlock*/
        /** building stack*/
        ///  module stack
        std::stack<Module_Stack_Element>  _module_stack;
        /// box stack to determine which hw element belong to box (box will be deprecated in next Kathryn version)
        std::stack<Box*>                  _box_stack;
        /// flow describe stack
        std::stack<FlowBlockBase*>        _flow_block_stacks[FLOW_ST_CNT];
        /// pattern flow block is subset of flowBlockStack
        Module*                           _global_module_ptr = nullptr;

    protected:
        /// module retrival
        Module*               get_top_module_ptr();
        Module_Stack_Element& get_target_module_ele();

        /// flow block stack handling
        void pop_flow_block       (FlowBlockBase* fb);
        void push_flow_block      (FlowBlockBase* fb);
        void detach_top_flow_block();
    public:

        FlowBlockBase* get_top_flow_block_base_ptr();
        FlowBlockBase* get_top_flow_block_base_ptr(FLOW_STACK_TYPE stackEnum);


    public:
        /**
         *
         * Hardware component handling fucntion
         *
         * */
        explicit ModelController();
        void     start() override;
        void     reset() override;
        void     clean() override;
        Module*  get_global_module_ptr();
        /** state register handling*/
        void on_sp_reg_init(CtrlFlowRegBase* ptr, SP_REG_TYPE reg_type);
        /** register handling*/
        void on_reg_init  (Reg* ptr);
        void on_reg_update(AsmNode* asmNode, Reg* src_reg);
        /** wire handling*/
        void on_wire_init  (Wire* ptr);
        void on_wire_update(AsmNode* asmNode, Wire* src_wire);
        /** exprMetas handling*/
        void on_expression_init(expression* ptr);
        /** on memBlock and its agent is updated*/
        void on_memBlk_init           (MemBlock* ptr);
        void on_memBlkEleHolder_update(AsmNode* asm_node,MemBlockEleHolder* src_holder);
        /** on nest init*/
        void on_nest_init  (nest* ptr);
        void on_nest_update(AsmNode* asmNode, nest* src_nest);
        /** value and pm value handling*/
        void on_value_init  (Val* ptr);
        void on_pmValue_init(PmVal* ptr);
        /** box handling*/
        void on_box_init       (Box* ptr);
        void on_box_end_init   (Box* ptr);
        void on_box_update     (AsmNode* asmNode, Box* box);
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
        void on_globalModule_init_component         ();
        void on_globalModule_init_designFlow        ();
        void on_globalModule_init_auxilaryComponent (); ///// for init some readonly logic for simulation trigger
        void on_globalModule_final_auxilaryComponent();
        ////// for nomal module
        void on_module_init_components    (Module* ptr);
        void on_module_end_init_components(Module* ptr);
        void on_module_init_designFlow    (Module* ptr); /** todo make design flow implement correctly*/
        void on_module_final              (Module* ptr);



        /**
         *
         * control flow block handler
         *
         * */

        void assign_flow_block_parent      (FlowBlockBase* fb);
        bool is_all_flow_stack_empty       ();
        bool is_flow_stack_empty           (FLOW_STACK_TYPE flow_stack_type);
        bool is_top_fb_belong_to_top_module();   /// sometimes module is declare inside flow block
        void try_purify_flow_stack         ();
        void on_attach_flowBlock           (FlowBlockBase* fb);
        void on_detach_flowBlock           (FlowBlockBase* fb);
        void on_attachAndDetach_intrSignal (INT_TYPE intType, Operable* sig);
        //Operable& on_get_check_next_pipblk_ready_signal();
        FLOW_BLOCK_TYPE get_top_pattern_flow_block_type();

        bool is_top_of_stack_belong_to_the_same_module(FLOW_STACK_TYPE a, FLOW_STACK_TYPE b);

        /** lock allocation*/
        void lock_allocation() {_hw_comp_alloc_lock = true;};
        void unlock_allocation(){_hw_comp_alloc_lock = false;};
        [[nodiscard]]
        bool is_allocation_lock() const{return _hw_comp_alloc_lock;}

        /** for debugging and model checking purpose*/
        std::string get_cur_model_stack_dbg();

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
