//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_FLOWBLOCK_BASE_H
#define KATHRYN_FLOWBLOCK_BASE_H

#include<memory>
#include<vector>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/abstract/operation.h"
#include "model/FlowBlock/abstract/nodes/node.h"
#include "model/FlowBlock/abstract/nodeWrap.h"
#include "model/FlowBlock/abstract/nodes/stateNode.h"


namespace kathryn {
    /** it is basic node that only have one event at a node */
    class Controller;
    enum FLOW_BLOCK_TYPE{
        SEQUENTIAL,
        PARALLEL_AUTO_SYNC,
        PARALLEL_NO_SYN,
        IF,
        ELIF,
        ELSE,
        WHILE,
        EXITWHILE,
        CONDWAIT,
        CLKWAIT,
        DO_WHILE,
        DUMMY_BLOCK,
        FLOW_BLOCK_COUNT
    };

    std::string FBT_to_string(FLOW_BLOCK_TYPE fbt);


    class FlowBlockBase {
    protected:
        std::vector<FlowBlockBase*> subBlocks;
        std::vector<Node*>          basicNodes;
        FLOW_BLOCK_TYPE             _type;
        Controller*                 ctrl = nullptr;
        bool                        lazyDeletedRequired = false;
        /*** for exit management*/
        bool                        areThereForceExit = false;
        PseudoNode*                 forceExitNode = nullptr;

        /** generate implicit subblock typically used with if and while block*/
        FlowBlockBase* genImplicitSubBlk(FLOW_BLOCK_TYPE defaultType);
        void genSumForceExitNode(std::vector<NodeWrap*>& nws);
    public:
        explicit FlowBlockBase(FLOW_BLOCK_TYPE type);
        virtual ~FlowBlockBase();

        /**
         * entrance to make controller interact with
         * */
        /** when basic behavior describe in flow block*/
        virtual void addElementInFlowBlock(Node* node){
            assert(node != nullptr);
            basicNodes.push_back(node);
        };
        /** when inside complex element such as sub flow block is finish, user must add here*/
        virtual void addSubFlowBlock(FlowBlockBase* subBlock){
            assert(subBlock != nullptr);
            subBlocks.push_back(subBlock);
        };
        /**
         * For custome block
         * */
        /** when everything is finish call this to get sumarisation*/
        virtual NodeWrap* sumarizeBlock() = 0;
        /*** communicator to controller*/
         virtual void onAttachBlock() = 0; //// it is supposed to acknowledge controller whether this block is declared
         virtual void onDetachBlock() = 0;
         /*** for module communicate with* */
          virtual void buildHwComponent() = 0;

        ////// getter/setter
        FLOW_BLOCK_TYPE getFlowType() const {return _type;}

        std::vector<FlowBlockBase*>& getSubBlocks(){
            return subBlocks;
        }
        /** lazy delete is the variable that tell controller whether
         * block should be pop from building stack when purifier is done
         * not when block is detach. Usually, It is used in if block
         * */
        bool isLazyDelete() const{
            return lazyDeletedRequired;
        }
        void setLazyDelete() {
            lazyDeletedRequired = true;
        }

        /** debug string*/
        std::string getFlowBlockDebugIdentValue() const{
            return "FB_TYPE_" + FBT_to_string(getFlowType()) +
                   "_@" + std::to_string((ull) this);
        }

    };

}

#endif //KATHRYN_FLOWBLOCK_BASE_H
