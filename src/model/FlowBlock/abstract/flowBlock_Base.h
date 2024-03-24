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
#include "flowIdentifiable.h"


namespace kathryn {
    /** it is basic node that only have one event at a node */
    class ModelController;
    enum FLOW_BLOCK_TYPE{
        SEQUENTIAL, /** seq to par_no_syn do not reorder it due to controller pattern checking*/
        PARALLEL_AUTO_SYNC,
        PARALLEL_NO_SYN,
        CIF,
        SIF,
        CSELIF,
        CSELSE,
        ZIF,
        ZELIF,
        ZELSE,
        CWHILE,
        SWHILE,
        EXITWHILE,
        CONDWAIT,
        CLKWAIT,
        DO_WHILE,
        PIPE_SENDER,
        PIPE_RECIEVER,
        DUMMY_BLOCK,
        FLOW_BLOCK_COUNT
    };

    enum FLOW_STACK_TYPE{
        FLOW_ST_BASE_STACK = 0,
        FLOW_ST_PATTERN_STACK = 1,    /**for seq par*/
        FLOW_ST_HEAD_COND_STACK = 2, /**for if only**/
        FLOW_ST_CNT = 3
    };

    enum FLOW_BLOCK_JOIN_POLICY{
        FLOW_JO_SUB_FLOW, /**for other block*/
        FLOW_JO_CON_FLOW, /** join flowblock for elif and else*/
        FLOW_JO_EXT_FLOW, /**extract this flow to be an basic node*/
    };

    struct FB_CTRL_COM_META{
        std::vector<FLOW_STACK_TYPE> _selFlowStack; //////// which stack for push/pop
        FLOW_BLOCK_JOIN_POLICY       _joinPolicy; ////// how to join with other block
        bool                         reqPurify = false;
    };


    std::string FBT_to_string(FLOW_BLOCK_TYPE fbt);

    extern int nextFbIdx;

    class FlowBlockBase: public FlowIdentifiable,
                         public ModelDebuggable,
                         public FlowSimEngineMaster,
                         public SimEngineInterface
                         {
    protected:

        struct sortEle{
            FlowBlockBase* fb;
            int order = -1;

            bool operator< (const sortEle& rhs) const{
                return order < rhs.order;
            }
        };

        int nextInputOrder = 0;
        /** flow element*/
        std::vector<FlowBlockBase*> _subBlocks;
        std::vector<int>            _subBlocksOrder; //// input order in this block
        std::vector<FlowBlockBase*> _conBlocks;
        std::vector<int>            _conBlocksOrder; //// input order in this block
        std::vector<Node*>          _basicNodes;
        std::vector<int>            _basicNodesOrder; //// input order in this block

        std::vector<FlowBlockBase*> _abandonedBlocks;  /// the flow block that have been extracted and push to this block

        /** status of node*/
        FLOW_BLOCK_TYPE             _type;
        ModelController*            ctrl = nullptr;
        bool                        lazyDeletedRequired = false;
        int                         _fbId;
        /** controller interactive element*/
        FB_CTRL_COM_META            _fbCtrlComMeta;
        /*** for exit management*/
        bool                        _areThereForceExit = false;
        PseudoNode*                 _forceExitNode     = nullptr;

        /** generate implicit subblock typically used with if and while block*/
        FlowBlockBase* genImplicitSubBlk(FLOW_BLOCK_TYPE defaultType);
        void           genSumForceExitNode(std::vector<NodeWrap*>& nws);
        Operable*      purifyCondition(Operable* rawOpr);
    public:
        explicit       FlowBlockBase(FLOW_BLOCK_TYPE  type,
                                     FB_CTRL_COM_META fbCtrlComMeta);
        virtual        ~FlowBlockBase();

        SimEngine*     getSimEngine() override{
                return static_cast<SimEngine*>(this);
        }
        /**
         * entrance to make controller interact with
         * */
        /** when basic behavior describe in flow block*/
        virtual void addElementInFlowBlock(Node* node){
            assert(node != nullptr);
            _basicNodes.push_back(node);
            _basicNodesOrder.push_back(nextInputOrder++);
        };
        /** when inside complex element such as sub flow block is finish, user must add here*/
        virtual void addSubFlowBlock(FlowBlockBase* subBlock){
            assert(subBlock != nullptr);
            _subBlocks.push_back(subBlock);
            _subBlocksOrder.push_back(nextInputOrder++);
        };
        /** add sub con block as consecutive block*/
        virtual void addConFlowBlock(FlowBlockBase* conBlock){
            assert(conBlock != nullptr);
            _conBlocks.push_back(conBlock);
            _conBlocksOrder.push_back(nextInputOrder++);
        }
        virtual void addAbandonFlowBlock(FlowBlockBase* abandonBlock){
            assert(abandonBlock != nullptr);
            _abandonedBlocks.push_back(abandonBlock);
        }
        /**
         * For custom block
         * */
        /** when everything is finish call this to get sumarisation*/
        virtual NodeWrap*   sumarizeBlock() = 0;
        /*** communicator to controller*/
        virtual void        onAttachBlock() = 0; //// it is supposed to acknowledge controller whether this block is declared
        virtual void        onDetachBlock() = 0;
        /*** for module controller build node and other elements*/
        virtual void        buildSubHwComponent();
        virtual void        buildHwComponent() = 0;
        ////// getter/setter
        FLOW_BLOCK_TYPE     getFlowType() const {return _type;}
        int                 getFlowBlockId() const{return _fbId;}
        std::vector<Node*>&
                            getBasicNode(){return _basicNodes;}
        std::vector<FlowBlockBase*>&
                            getSubBlocks(){return _subBlocks;}
        std::vector<FlowBlockBase*>&
                            getConBlocks(){return _conBlocks;}
        /** lazy delete is the variable that tell controller whether
         * block should be pop from building stack when purifier is done
         * not when block is detach. Usually, It is used in if block
         * */
        bool                isLazyDelete() const{ return lazyDeletedRequired; }
        void                setLazyDelete()     { lazyDeletedRequired = true;}
        void                unsetLazyDelete()   {lazyDeletedRequired = false;}
        /** controller communication*/
        [[nodiscard]]
        std::vector<FLOW_STACK_TYPE> getSelFbStack() const {return _fbCtrlComMeta._selFlowStack;}
        [[nodiscard]]
        FLOW_BLOCK_JOIN_POLICY       getJoinFbPol()  const {return _fbCtrlComMeta._joinPolicy;  }
        [[nodiscard]]
        bool                         getPurifyReq()  const {return _fbCtrlComMeta.reqPurify;    }

        /** debug method*/
        std::string getMdDescribeRecur() {
            std::string ret = "----------- sub Block --------\n";
            for (auto sb : _subBlocks){
                ret += sb->getMdDescribe();
                ret += "\n";
            }
            return ret;
        }

        void addMdLogRecur(MdLogVal *mdLogVal){
            if (_subBlocks.empty())
                return;
            mdLogVal->addVal("-----sub block------");
            for (auto sb: _subBlocks){
                auto subStruct = mdLogVal->makeNewSubVal();
                sb->addMdLog(subStruct);
            }
        }

        [[nodiscard]]std::string getMdIdentVal() override{
            return FBT_to_string(getFlowType()) + "_blockId_" + std::to_string(_fbId);
        }

        std::vector<sortEle> sortSubAndConFbInOrder();
        void beforePrepareSim(FlowSimEngine::FLOW_Meta_afterMf simMeta) override;

    };

}

#endif //KATHRYN_FLOWBLOCK_BASE_H
