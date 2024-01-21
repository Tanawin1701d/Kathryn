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
    class ModelController;
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

    extern int nextFbIdx;

    class FlowBlockBase: public ModelDebuggable,
                         public FlowSimulatable{
    protected:
        std::vector<FlowBlockBase*> subBlocks;
        std::vector<Node*>          basicNodes;
        FLOW_BLOCK_TYPE             _type;
        ModelController*            ctrl = nullptr;
        bool                        lazyDeletedRequired = false;
        int                         _fbId;
        /*** for exit management*/
        bool                        areThereForceExit = false;
        PseudoNode*                 forceExitNode = nullptr;

        /** generate implicit subblock typically used with if and while block*/
        FlowBlockBase* genImplicitSubBlk(FLOW_BLOCK_TYPE defaultType);
        void           genSumForceExitNode(std::vector<NodeWrap*>& nws);
    public:
        explicit  FlowBlockBase(FLOW_BLOCK_TYPE type);
        virtual  ~FlowBlockBase();
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
         * For custom block
         * */
        /** when everything is finish call this to get sumarisation*/
        virtual NodeWrap*   sumarizeBlock() = 0;
        /*** communicator to controller*/
        virtual void        onAttachBlock() = 0; //// it is supposed to acknowledge controller whether this block is declared
        virtual void        onDetachBlock() = 0;
        /*** for module controller build node and other elements*/
        virtual void        buildHwComponent() = 0;
        ////// getter/setter
        FLOW_BLOCK_TYPE     getFlowType() const {return _type;}
        int                 getFlowBlockId() const{return _fbId;}
        std::vector<FlowBlockBase*>& getSubBlocks(){return subBlocks;}
        /** lazy delete is the variable that tell controller whether
         * block should be pop from building stack when purifier is done
         * not when block is detach. Usually, It is used in if block
         * */
        bool isLazyDelete() const{ return lazyDeletedRequired; }
        void setLazyDelete() { lazyDeletedRequired = true;}

        /** debug method*/
        std::string getMdDescribeRecur() {
            std::string ret = "----------- sub Block --------\n";
            for (auto sb : subBlocks){
                ret += sb->getMdDescribe();
                ret += "\n";
            }
            return ret;
        }

        void addMdLogRecur(MdLogVal *mdLogVal){
            if (subBlocks.empty())
                return;
            mdLogVal->addVal("-----sub block------");
            for (auto sb: subBlocks){
                auto subStruct = mdLogVal->makeNewSubVal();
                sb->addMdLog(subStruct);
            }
        }

        [[nodiscard]]std::string getMdIdentVal() override{
            return FBT_to_string(getFlowType()) + "_blockId_" + std::to_string(_fbId);
        }

    };

}

#endif //KATHRYN_FLOWBLOCK_BASE_H
