//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_CONTROLLER_H
#define KATHRYN_CONTROLLER_H

#include <stack>
#include <memory>

#include "model/hwComponent/module/module.h"
#include "model/FlowBlock/abstract/nodes/asmNode.h"
#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/par/par.h"
#include "model/FlowBlock/cond/if.h"
#include "model/FlowBlock/cond/elif.h"
#include "model/FlowBlock/loop/cwhile.h"
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



    class Controller {

    private:
        bool hwCompAllocLock = true; /** this is used to indicate whether make<> is used or not only make<> can unlock*/
        /** building stack*/
        ////// module stack
        std::stack<Module_Stack_Element>  moduleStack;

        ////// flow describe stack
        std::stack<FlowBlockBase*>        flowBlockStack; //// collect stack of all flowblock
        std::stack<FlowBlockBase*>        patternFlowBlockStack; //// stack of only seq or parallel
        std::stack<FlowBlockIf*>          ifBlockStack; /// stack of only cif or celif celse
        /////// pattern flow block is subset of flowBlockStack

    protected:
        /** get module that response we now consider*/
        Module* getTargetModulePtr();
        Module_Stack_Element& getTargetModuleEle();
        FlowBlockBase* getTopFlowBlock();

        static void tryPopFbBaseFromStack(FlowBlockBase* flowPtr,
                                   std::stack<FlowBlockBase*>& srcSt);
        static void tryPopFbIfFromStack(FlowBlockBase* flowPtr,
                                 std::stack<FlowBlockIf*>& srcSt);
        void tryPopCtrlFlowFromAllStack(FlowBlockBase* flowPtr);

        bool isAllFlowStackEmpty() {return  flowBlockStack.empty() &&
                                            patternFlowBlockStack.empty() &&
                                            ifBlockStack.empty();}

        void removeLazyFbFromTopStack();

        ////  check that stack rule before add or remove any event

    public:

        /**
         * event handling function
         * */

        /** state register handling*/
        void on_state_reg_init(StateReg* ptr);
        void on_sync_reg_init(SyncReg* ptr);
        void on_cond_wait_reg_init(CondWaitStateReg* ptr);
        void on_cycle_wait_reg_init(CycleWaitStateReg* ptr);
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
        void on_globalModule_init_component(Module* globalMod);
        void on_module_init_components(Module* ptr);
        void on_module_init_designFlow(Module* ptr); /** todo make design flow implement correctly*/
        void on_module_final(Module* ptr);

        /** control flow block handler*/
        void purifyFlowStack();
        void on_attach_flowBlock(FlowBlockBase* fb);
        void on_detach_flowBlock(FlowBlockBase* fb);

        void on_attach_flowBlock_if(FlowBlockIf* fb);

        void on_attach_flowBlock_elif(FlowBlockElif* fb);
        void on_detach_flowBlock_elif(FlowBlockElif* fb);

        FLOW_BLOCK_TYPE get_top_pattern_flow_block_type();

        /** lock allocation*/
        void lockAllocation() {hwCompAllocLock = true;};
        void unlockAllocation(){hwCompAllocLock = false;};
        bool isAllocationLock() const{return hwCompAllocLock;}

    };

    /** this is entrace for every device to com_init with controller*/
    Controller* getControllerPtr();
    void        freeControllerPtr();



}

#endif //KATHRYN_CONTROLLER_H
