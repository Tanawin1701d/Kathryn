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
        ModulePtr md;
        MODULE_BUILDING_STATE state;
    };



    class Controller {

    private:
        bool hwCompAllocLock = true; /** this is used to indicate whether make<> is used or not only make<> can unlock*/
        /** building stack*/
        std::stack<Module_Stack_Element>  moduleStack;
        std::stack<FlowBlockBasePtr>      flowBlockStack; //// collect stack of all flowblock
        std::stack<FlowBlockBasePtr>      patternFlowBlockStack; //// stack of only seq or parallel
        std::stack<FlowBlockBasePtr>      condStlessFlowBlockStack; /// stack of only cif or celif celse
        /////// pattern flow block is subset of flowBlockStack

    protected:
        /** get module that response we now consider*/
        ModulePtr getTargetModulePtr();

        void tryPopFromStack(const FlowBlockBasePtr& flowPtr, std::stack<FlowBlockBasePtr>& srcSt);
        void tryPopCtrlFlowFromAllStack(const FlowBlockBasePtr& flowPtr);

        bool isAllFlowStackEmpty() {return  flowBlockStack.empty() &&
                                            patternFlowBlockStack.empty() &&
                                            condStlessFlowBlockStack.empty();}


    public:

        /**
         * event handling function
         * */

        /** register handling*/
        void on_reg_init(const RegPtr& ptr);
        void on_reg_update(const std::shared_ptr<UpdateEvent>& upEvent);
        /** wire handling*/
        void on_wire_init(const WirePtr& ptr);
        void on_wire_update(const std::shared_ptr<UpdateEvent>& upEvent);
        /** expression handling*/
        void on_expression_init(const expressionPtr& ptr);
        /** value handling*/
        void on_value_init(const ValPtr& ptr);
        /** module handling*/
        void on_module_init_components(const ModulePtr& ptr);
        void on_module_init_designFlow(ModulePtr ptr); /** todo make design flow implement correctly*/
        void on_module_final(ModulePtr ptr);

        /** control flow block handler*/
        void on_attach_flowBlock(const FlowBlockBasePtr& fb);
        void on_detach_flowBlock(const FlowBlockBasePtr& fb);

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
