//
// Created by tanawin on 30/11/2566.
//

#ifndef KATHRYN_CONTROLLER_H
#define KATHRYN_CONTROLLER_H

#include <stack>
#include <memory>

#include "model/hwComponent/module/module.h"
#include "model/FlowBlock/abstract/flowBlock_Base.h"


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
        std::stack<Module_Stack_Element>  moduleStack;
        std::stack<FlowBlockBase*>        flowBlockStack; //// collect stack of all flowblock
        std::stack<FlowBlockBase*>        patternFlowBlockStack; //// stack of only seq or parallel
        std::stack<FlowBlockBase*>        condStlessFlowBlockStack; /// stack of only cif or celif celse
        /////// pattern flow block is subset of flowBlockStack

    protected:
        /** get module that response we now consider*/
        Module* getTargetModulePtr();
        Module_Stack_Element& getTargetModuleEle();
        FlowBlockBase* getTopFlowBlock();

        void tryPopFromStack(FlowBlockBase* flowPtr, std::stack<FlowBlockBase*>& srcSt);
        void tryPopCtrlFlowFromAllStack(FlowBlockBase* flowPtr);

        bool isAllFlowStackEmpty() {return  flowBlockStack.empty() &&
                                            patternFlowBlockStack.empty() &&
                                            condStlessFlowBlockStack.empty();}


    public:

        /**
         * event handling function
         * */

        /** register handling*/
        void on_reg_init(Reg* ptr);
        void on_reg_update(UpdateEvent* upEvent);
        /** wire handling*/
        void on_wire_init(Wire* ptr);
        void on_wire_update(UpdateEvent* upEvent);
        /** expression handling*/
        void on_expression_init(expression* ptr);
        /** value handling*/
        void on_value_init(Val* ptr);
        /** module handling*/
        void on_module_init_components(Module* ptr);
        void on_module_init_designFlow(Module* ptr); /** todo make design flow implement correctly*/
        void on_module_final(Module* ptr);

        /** control flow block handler*/
        void on_attach_flowBlock(FlowBlockBase* fb);
        void on_detach_flowBlock(FlowBlockBase* fb);

        /** lock allocation*/
        void lockAllocation() {hwCompAllocLock = true;};
        void unlockAllocation(){hwCompAllocLock = false;};
        bool isAllocationLock() const{return hwCompAllocLock;}

    };


    typedef std::shared_ptr<Controller> ControllerPtr;
    /** this is entrace for every device to com_init with controller*/
    ControllerPtr getControllerPtr();
    void          freeControllerPtr();



}

#endif //KATHRYN_CONTROLLER_H
