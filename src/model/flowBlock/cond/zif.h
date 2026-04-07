//
// Created by tanawin on 2/2/2567.
//

#ifndef KATHRYN_ZIF_H
#define KATHRYN_ZIF_H

#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"
#include "zifClassAsm.h"

#define zif(expr) for(auto kathrynBlock = new FlowBlockZIF(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{



    class FlowBlockZIF: public FlowBlockBase,
                        public LoopStMacro{
    private:
        bool lastZelifDetected = false;
        Operable* purifiedCurCond  = nullptr;
        std::vector<Operable*> prevFalses;

        //// node will  be delete in this stage
        std::vector<ZifClassAsm*> _assignMetas;



    public:

        explicit FlowBlockZIF(Operable& cond);
        ~FlowBlockZIF() override;

        /** for controller add the local element to this sub block*/
        void add_basic_node(Node* node) override;
        void add_sub_flow_block(FlowBlockBase* subBlock) override;
        void add_con_flow_block(FlowBlockBase* fb) override;
        void add_intr_signal(INT_TYPE type, Operable* signal) override;
        NodeWrap* sumarize_block() override;
        /** on this block is start interact to controller*/
        void on_attach_block() override;
        /** on leave this block*/
        void on_detach_block() override;
        /** for module to build hardware component*/
        void build_hw_component() override;
        /** get describe*/
        std::string get_md_describe() override;
        void add_md_log(MdLogVal *mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
        /** extracted system*/
        std::vector<AsmNode*> extract() override;
    };



}

#endif //KATHRYN_ZIF_H
