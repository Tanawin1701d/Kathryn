//
// Created by tanawin on 8/12/2566.
//

#ifndef KATHRYN_ELIF_H
#define KATHRYN_ELIF_H
#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"

#define celif(expr) for(auto kathrynBlock = new FlowBlockElif(expr); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define celse for(auto kathrynBlock = new FlowBlockElif(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())

namespace kathryn{

    class FlowBlockElif: public FlowBlockBase, public LoopStMacro{
    private:
        NodeWrap* resultNodeWrapper = nullptr;
        Operable* _cond = nullptr;

    public:
        explicit FlowBlockElif(Operable& cond);
        explicit FlowBlockElif();

        ~FlowBlockElif();

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        NodeWrap* sumarizeBlock() override;
        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

        [[nodiscard]]
        Operable* getCondition() const{
            return _cond;
        }


    };


}

#endif //KATHRYN_ELIF_H
