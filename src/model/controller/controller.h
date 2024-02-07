//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_MODEL_CONTROLLER_H
#define KATHRYN_MODEL_CONTROLLER_H

#include <stack>
#include <memory>

#include "model/hwComponent/module/module.h"
#include "model/FlowBlock/abstract/nodes/asmNode.h"
#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/seq/seq.h"
#include "model/FlowBlock/par/par.h"
#include "model/FlowBlock/loop/swhile.h"
#include "model/FlowBlock/loop/cwhile.h"
#include "model/FlowBlock/loop/cbreak.h"
#include "model/FlowBlock/cond/zif.h"
#include "model/FlowBlock/cond/zelif.h"
#include "model/FlowBlock/cond/if.h"
#include "model/FlowBlock/cond/elif.h"
#include "model/FlowBlock/abstract/spReg/stateReg.h"
#include "model/FlowBlock/abstract/spReg/syncReg.h"


namespace kathryn {

    enum MODULE_BUILDING_STATE{
        MODULE_COMPONENT_CONSTRUCT,
        MODULE_DESIGN_FLOW_CONSTRUCT,
        MODULE_FINISHED_CONSTRUCT
    };

    struct Module_Stack_Element{
        Module* md;
        MODULE_BUILDING_STATE state;
    };



    class ModelController {

    private:
        bool hwCompAllocLock = true; /** this is used to indicate whether make<> is used or not only make<> can unlock*/
        /** building stack*/
        ////// module stack
        std::stack<Module_Stack_Element>  moduleStack;
        ////// flow describe stack
        std::stack<FlowBlockBase*> flowBlockStacks[FLOW_ST_CNT];
        /////// pattern flow block is subset of flowBlockStack
        Module* globalModulePtr = nullptr;

    protected:
        /** get module that response we now consider*/
        Module* getTargetModulePtr();
        Module_Stack_Element& getTargetModuleEle();

       FlowBlockBase* getTopFlowBlockBase();
       void           popFlowBlock(FlowBlockBase* fb);
       void           pushFlowBlock(FlowBlockBase* fb);
       void           detachTopFlowBlock();


    public:

        /**
         * event handling function
         * */
        explicit ModelController();
        void reset();
        Module* getGlobalModule();
        /** state register handling*/
        void on_sp_reg_init(CtrlFlowRegBase* ptr, SP_REG_TYPE regType);
        /** register handling*/
        void on_reg_init(Reg* ptr);
        void on_reg_update(AssignMeta* asmMeta, Reg* srcReg);
        /** wire handling*/
        void on_wire_init(Wire* ptr);
        void on_wire_update(AssignMeta* asmMeta, Wire* srcWire);
        /** exprMetas handling*/
        void on_expression_init(expression* ptr);
        /** value handling*/
        void on_value_init(Val* ptr);
        /** module handling*/
        void on_globalModule_init_component();
        void on_module_init_components(Module* ptr);
        void on_module_init_designFlow(Module* ptr); /** todo make design flow implement correctly*/
        void on_module_final(Module* ptr);

        /** control flow block handler*/
        bool           isAllFlowStackEmpty();
        void tryPurifyFlowStack();
        void on_attach_flowBlock(FlowBlockBase* fb);
        void on_detach_flowBlock(FlowBlockBase* fb);

        FLOW_BLOCK_TYPE get_top_pattern_flow_block_type();

        /** lock allocation*/
        void lockAllocation() {hwCompAllocLock = true;};
        void unlockAllocation(){hwCompAllocLock = false;};
        [[nodiscard]]
        bool isAllocationLock() const{return hwCompAllocLock;}

    };

    /** this is entrace for every device to com_init with controller*/
    ModelController* getControllerPtr();
    Module*          getGlobalModulePtr();
    void        freeControllerPtr();

}

#endif //KATHRYN_ModelCONTROLLER_H
