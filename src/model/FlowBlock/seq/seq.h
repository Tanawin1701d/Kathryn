//
// Created by tanawin on 5/12/2566.
//

#ifndef KATHRYN_SEQ_H
#define KATHRYN_SEQ_H


#include "model/FlowBlock/abstract/flowBlock_Base.h"
#include "model/FlowBlock/abstract/loopStMacro.h"


namespace kathryn {

    class SeqenceMeta{
    public:
        /**assignment block*/
        Node* _simpleAsm = nullptr;
        FlowBlockBase* _subBlock = nullptr;
        NodeWrapper* nodeWrapper = nullptr;
        bool isGenHwYet          = false;

        explicit SeqenceMeta(Node* simpleAsm):_simpleAsm(simpleAsm){}
        explicit SeqenceMeta(FlowBlockBase* subBlock): _subBlock(subBlock){}

        void genHw();

        NodeWrapper* getNodeWrapper() const{
            assert(isGenHwYet);
            return nodeWrapper;
        }

        ~SeqenceMeta(){
            /** subBlock master node will handle it */
            delete _simpleAsm;
            delete nodeWrapper;
        }

    };

    class Seq : public FlowBlockBase, public LoopStMacro{
    private:

        std::vector<SeqenceMeta> _subSeqMetas;

    public:

        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        NodeWrapper* sumarizeBlock() override;

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;

    };


}

#endif //KATHRYN_SEQ_H
