//
// Created by tanawin on 7/2/26.
//

#ifndef MODEL_FLOWBLOCK_SEQ_SEQELE_H
#define MODEL_FLOWBLOCK_SEQ_SEQELE_H

#include "model/flowBlock/abstract/nodeWrap.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"

namespace kathryn{

    class SequenceEle{
    public:
        /**node and flow block*/
        OprNode*          _intRstNode = nullptr;
        OprNode*          _holdNode   = nullptr;


        /**state representation*/
        virtual ~SequenceEle() = default;

        //////// generate structure command
        virtual void               genNode         (CLOCK_MODE cm) = 0; /// clock mode is used only when state node is used
        //////// set extension node command
        virtual void               setIdentStateId     (ull masterIdx, int subIdx) const = 0;
                void               setIntReset         (OprNode* intResetNode    );
                void               setHoldNode         (OprNode* holdNode        );
        virtual void               addToCycleDet       (NodeWrapCycleDet& deter  ) const = 0;
        virtual void               assignDependDent    (SequenceEle* predecessor ) const = 0;
        virtual void               assignIntStart      (OprNode* intStartNode    ) = 0;
        //////// get/check command
                OprNode*           getIntResetNode     () const {return _intRstNode;}
                OprNode*           getHoldNode         () const {return _holdNode;}
        virtual Node*              getStateFinishIden  () const = 0;
        virtual std::vector<Node*> getEntranceNodes    () = 0;
        virtual bool               isThereForceExitNode() const = 0;
        virtual Node*              getForceExitNode    () const = 0;
        virtual bool               isNodeWrap          () const = 0;
        virtual NodeWrap*          getNodeWrap         () const = 0;
        virtual bool               isBasicNode         () const = 0;
        virtual StateNode*         getBasicNode        () const = 0;
        virtual std::string        getDescribe         () = 0;
        virtual void               addToSystemNodes    (std::vector<Node*>& sysNode) = 0;

        virtual void               assignDependDent    (Node* activatorNode) const = 0;

    };

    /**
     * the basic element wrapper
     */
    class SequenceEleBasic: public SequenceEle{
        AsmNode*   _asmNode    = nullptr;
        StateNode* _stateNode  = nullptr;
    public:
        explicit SequenceEleBasic(Node* asmNode);
        ~SequenceEleBasic() override;

        void               genNode         (CLOCK_MODE cm)                       override;
        void               setIdentStateId (ull masterIdx, int subIdx) const     override;
        void               addToCycleDet       (NodeWrapCycleDet& deter) const   override;
        void               assignDependDent    (SequenceEle* predecessor) const  override;
        void               assignIntStart      (OprNode* intStartNode)           override;
        Node*              getStateFinishIden  () const                          override;
        std::vector<Node*> getEntranceNodes    ()                                override;
        bool               isThereForceExitNode() const                          override;
        Node*              getForceExitNode    () const                          override;
        bool               isNodeWrap          () const                          override;
        NodeWrap*          getNodeWrap         () const                          override;
        bool               isBasicNode         () const                          override;
        StateNode*         getBasicNode        () const                          override;
        std::string        getDescribe         ()                                override;
        void               addToSystemNodes    (std::vector<Node*>& sysNode)     override;

        /////// extension for custom dependency
        void               assignDependDent    (Node* activatorNode) const override;

    };

    /**
     * the flow block wrapper
     */
    class SequenceEleFlowBlock: public SequenceEle{
        FlowBlockBase* _subBlock    = nullptr;
        NodeWrap*      _complexNode = nullptr;
    public:
        explicit SequenceEleFlowBlock(FlowBlockBase* fbBase);

        void               genNode         (CLOCK_MODE cm)                       override;
        void               setIdentStateId (ull masterIdx, int subIdx) const     override;
        void               addToCycleDet       (NodeWrapCycleDet& deter) const   override;
        void               assignDependDent    (SequenceEle* predecessor) const  override;
        void               assignIntStart      (OprNode* intStartNode)           override;
        Node*              getStateFinishIden  () const                          override;
        std::vector<Node*> getEntranceNodes    ()                                override;
        bool               isThereForceExitNode() const                          override;
        Node*              getForceExitNode    () const                          override;
        bool               isNodeWrap          () const                          override;
        NodeWrap*          getNodeWrap         () const                          override;
        bool               isBasicNode         () const                          override;
        StateNode*         getBasicNode        () const                          override;
        std::string        getDescribe         ()                                override;
        void               addToSystemNodes    (std::vector<Node*>& sysNode)     override;

        /////// extension for custom dependency
        void               assignDependDent    (Node* activatorNode) const override;

    };

}

#endif //MODEL_FLOWBLOCK_SEQ_SEQELE_H