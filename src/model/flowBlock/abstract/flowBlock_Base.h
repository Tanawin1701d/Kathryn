//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_FLOWBLOCK_BASE_H
#define KATHRYN_FLOWBLOCK_BASE_H


#define intrReset( expr  )   kathrynBlock->addIntSignal(INT_RESET, &expr);
#define intrStart( expr  )   kathrynBlock->addIntSignal(INT_START, &expr);
#define holdBlk( expr  )     kathrynBlock->addHoldSignal(&expr);
#define intrRstAndStart( expr ) intrReset(expr) intrStart(expr)
#define exposeBlk( exVar )   exVar = kathrynBlock;
#define track( name  )       kathrynBlock->setZepTrackName(#name);
#define strack( name )       kathrynBlock->setZepTrackName(name);
#define markJoinMaster       kathrynBlock->setJoinMaster();

#include<memory>
#include<vector>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/abstract/operation.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodeWrap.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"
#include "model/flowBlock/abstract/nodes/logicNode.h"
#include "sim/modelSimEngine/flowBlock/flowBaseSim.h"

#include "flowIdentifiable.h"
#include "flowBlockRegister.h"

#include "logicHelper.h"



namespace kathryn {
    /** it is basic node that only have one event at a node */
    class ModelController;

    enum FLOW_STACK_TYPE{
        FLOW_ST_BASE_STACK      = 0,
        FLOW_ST_PATTERN_STACK   = 1,    /**for seq par*/
        FLOW_ST_HEAD_COND_STACK = 2, /**for if only**/
        FLOW_ST_PIP_BLK         = 3,
        FLOW_ST_CNT             = 4
    };

    enum FLOW_BLOCK_JOIN_POLICY{
        FLOW_JO_SUB_FLOW, /**for other block*/
        FLOW_JO_CON_FLOW, /** join flowblock for elif and else*/
        FLOW_JO_EXT_FLOW, /**extract this flow to be an basic node*/
    };

    enum INT_TYPE{
        INT_RESET = 0,
        INT_START = 1,
        INT_CNT = 2,
    };

    struct FB_CTRL_COM_META{
        std::vector<FLOW_STACK_TYPE> _selFlowStack; //////// which stack for push/pop
        FLOW_BLOCK_JOIN_POLICY       _joinPolicy; ////// how to join with other block
        bool                         reqPurify = false;
    };

    extern int nextFbIdx;

    class FlowBlockBase: public FlowIdentifiable,
                         public ModelDebuggable,
                         public FlowSimEngineInterface
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
        std::vector<Node*>          _sysNodes;

        std::vector<FlowBlockBase*> _abandonedBlocks;  /// the flow block that have been extracted and push to this block

        /**  basic interrupt signals*/
        std::vector<Operable*>        intSignals[INT_CNT];
        OprNode*                      intNodes  [INT_CNT]; //// the sum of allnode
        /** basic  hold signals*/
        /** the hold signal supposed to hold the state without execute it*/
        std::vector<Operable*>        holdSignals;
        OprNode*                      holdNode = nullptr;
        /** CLOCK MODE*/
        CLOCK_MODE                    clkMode;





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
        /*** flow block sim engine*/
        FlowBaseSimEngine*          _flowSimEngine     = nullptr;

        /** generate implicit subblock typically used with if and while block*/
        FlowBlockBase* genImplicitSubBlk(FLOW_BLOCK_TYPE defaultType);
        /** generate sum of force exit note (the global variable)*/
        void           genSumForceExitNode(std::vector<NodeWrap*>& nws);
        /** interrupt node*/
        void           fillIntRstSignalToChild(); //// use for pass the data when build
        void           genIntNode();
        bool           isThereIntStart();
        bool           isThereIntRst();
        /** holding system*/
        void           fillHoldSignalToChild();
        void           genHoldNode();
        bool           isThereHold();
        /** clock mode*/
        void           setClockMode(CLOCK_MODE mode);
        CLOCK_MODE     getClockMode() const {return clkMode;}

        ///////////////////////////////////////
        Operable*      purifyCondition(Operable* rawOpr);
        FlowBlockBase* scanMasterJoinSubBlock();
    public:
        explicit       FlowBlockBase(FLOW_BLOCK_TYPE  type,
                                     FB_CTRL_COM_META fbCtrlComMeta);
        virtual        ~FlowBlockBase();

        Operable*      genIntSumSignal(bool isAndCond, INT_TYPE intrType); //// it pool all condition to single signal

        FlowBaseSimEngine* getSimEngine() override{
                return _flowSimEngine;
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
        /** system node is the node used to monitor by hybrid profiler*/
        virtual void addSysNode(Node* node){
            assert(node != nullptr);
            _sysNodes.push_back(node);
        }
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

        virtual void addIntSignal(INT_TYPE type, Operable* signal){
            assert(signal != nullptr); assert(type < INT_CNT); assert(signal->getOperableSlice().getSize() == 1);
            intSignals[type].push_back(signal);
        }

        void fillIntResetToNodeIfThere(Node* nd){
            if (intNodes[INT_RESET] != nullptr){
                nd->setInterruptReset(intNodes[INT_RESET]);
            }
        }

        /** hold entrance*/
        virtual void addHoldSignal(Operable* signal){
            assert(signal != nullptr);
            holdSignals.push_back(signal);
        }

        void fillHoldToNodeIfThere(Node* nd){
            if (holdNode != nullptr){
                nd->setHold(holdNode);
            }
        }

        std::vector<sortEle> sortSubAndConFbInOrder();
        void                 overrideClockModeInAllAsmNodes();
        /**
         * For custom block
         * */
        /** when everything is finish call this to get sumarisation*/
        virtual NodeWrap*   sumarizeBlock() = 0;
        /*** communicator to controller*/
        virtual void        onAttachBlock() = 0; //// it is supposed to acknowledge controller whether this block is declared
        virtual void        onDetachBlock() = 0;
        /*** for module controller build node and other elements*/
        virtual void        buildHwMaster();
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
        std::vector<Node*>&
                            getSysNodes(){return _sysNodes;}
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
    };

}

#endif //KATHRYN_FLOWBLOCK_BASE_H
