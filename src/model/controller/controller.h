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
#include "model/flowBlock/seq/seq.h"
#include "model/flowBlock/par/par.h"
#include "model/flowBlock/loop/whileBase.h"
#include "model/flowBlock/loop/doWhileBase.h"
#include "model/flowBlock/loop/cbreak.h"
#include "model/flowBlock/cond/zif.h"
#include "model/flowBlock/cond/zelif.h"
#include "model/flowBlock/cond/if.h"
#include "model/flowBlock/cond/elif.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/spReg/syncReg.h"
#include "model/flowBlock/time/wait.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "model/flowBlock/pipeline/pipeWrapper.h"

#include "util/type/typeConv.h"
#include "util/str/strUtil.h"



namespace kathryn {

    enum MODULE_BUILDING_STATE{
        MODULE_INIT, /** module element is declared but not yet init design flow only for element initialization*/
        MODULE_END_GLOB_DECLARE, /** module end init element*/
        MODULE_INIT_DESIGN_FLOW, /** module start init design flow*/
        MODULE_END /** module finalize*/

    };

    struct Module_Stack_Element{
        Module* md;
        MODULE_BUILDING_STATE state;
    };


    class MainControlable;

    class ModelController : public MainControlable {

    private:
        bool hwCompAllocLock = true; /** this is used to indicate whether make<> is used or not only make<> can unlock*/
        /** building stack*/
        ////// module stack
        std::stack<Module_Stack_Element>  moduleStack;
        ////// box stack to determine which hw element belong to stack
        std::stack<Box*> boxStack;
        ////// trace that the declare element should belong to box
        Assignable* boxLock = nullptr; ///// the first element that in push in boxLock must be user but other is auto build element
        ////// flow describe stack
        std::stack<FlowBlockBase*> flowBlockStacks[FLOW_ST_CNT];
        /////// pattern flow block is subset of flowBlockStack
        Module* globalModulePtr = nullptr;

        PipeController pipeCtrl;

    protected:
        /** get module that response we now consider*/
        Module* getTopModulePtr();
        Module_Stack_Element& getTargetModuleEle();

       FlowBlockBase* getTopFlowBlockBase();
       void           popFlowBlock(FlowBlockBase* fb);
       void           pushFlowBlock(FlowBlockBase* fb);
       void           detachTopFlowBlock();

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
        Module* getGlobalModule();
        /** state register handling*/
        void on_sp_reg_init(CtrlFlowRegBase* ptr, SP_REG_TYPE regType);
        /** register handling*/
        void on_reg_init(Reg* ptr);
        void on_reg_update(AsmNode* asmNode, Reg* srcReg);
        /** wire handling*/
        void on_wire_init(Wire* ptr);
        void on_wire_update(AsmNode* asmNode, Wire* srcWire);
        /** exprMetas handling*/
        void on_expression_init(expression* ptr);
        /** on memBlock and its agent is updated*/
        void on_memBlk_init(MemBlock* ptr);
        void on_memBlkEleHolder_update(AsmNode* asmNode,MemBlockEleHolder* srcHolder);
        /** on nest init*/
        void on_nest_init(nest* ptr);
        void on_nest_update(AsmNode* asmNode, nest* srcNest);
        /** value handling*/
        void on_value_init(Val* ptr);
        /** box handling*/
        void on_box_init(Box* ptr);
        void on_box_end_init(Box* ptr);
        void on_box_update(AsmNode* asmNode, Box* box);
        void on_chk_and_lock_belongBlk(Assignable* asb, Operable* opr);
        void on_chk_and_release_blk(Assignable* asb);

        /**
         *
         * module handling
         *
         * */

        ////// for global component
        void on_globalModule_init_component();
        void on_globalModule_init_designFlow();
        ////// for basic component
        void on_module_init_components(Module* ptr);
        void on_module_end_init_components(Module* ptr);
        void on_module_init_designFlow(Module* ptr); /** todo make design flow implement correctly*/
        void on_module_final(Module* ptr);

        /**
         *
         * control flow block handler
         *
         * */

        void assignFlowBlockParent(FlowBlockBase* fb);
        bool isAllFlowStackEmpty();
        bool isTopFbBelongToTopModule();
        void tryPurifyFlowStack();
        void on_attach_flowBlock(FlowBlockBase* fb);
        void on_detach_flowBlock(FlowBlockBase* fb);
        void on_attachAndDetach_intrSignal(INT_TYPE intType, Operable* sig);
        FLOW_BLOCK_TYPE get_top_pattern_flow_block_type();

        /** lock allocation*/
        void lockAllocation() {hwCompAllocLock = true;};
        void unlockAllocation(){hwCompAllocLock = false;};
        [[nodiscard]]
        bool isAllocationLock() const{return hwCompAllocLock;}

        /** for debugging and model checking purpose*/
        std::string getCurModelStack();

        /**
         *
         * pipeline controller
         *
         * */

        PipeController* getPipeCtrl(){return &pipeCtrl;}

    };

    /** this is entrace for every device to com_init with controller*/
    ModelController* getControllerPtr();
    Module*          getGlobalModulePtr();
    void             freeControllerPtr();

}

#endif //KATHRYN_ModelCONTROLLER_H
