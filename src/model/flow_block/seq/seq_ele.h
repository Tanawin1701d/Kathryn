//
// Created by tanawin on 7/2/26.
//

#ifndef MODEL_FLOWBLOCK_SEQ_SEQELE_H
#define MODEL_FLOWBLOCK_SEQ_SEQELE_H

#include "model/flow_block/abstract/node_wrap.h"
#include "model/flow_block/abstract/sp_reg/state_reg.h"
#include "model/flow_block/abstract/nodes/node.h"
#include "model/flow_block/abstract/nodes/state_node.h"

namespace kathryn{

    class SequenceEle{
    public:
        /**node and flow block*/
        OprNode*          _intRstNode = nullptr;
        OprNode*          _holdNode   = nullptr;


        /**state representation*/
        virtual ~SequenceEle() = default;

        //////// generate structure command
        virtual void               gen_node         (CLOCK_MODE cm) = 0; /// clock mode is used only when state node is used
        //////// set extension node command
        virtual void               set_ident_state_id     (ull master_idx, int sub_idx) const = 0;
                void               set_int_reset         (OprNode* int_reset_node    );
                void               set_hold_node         (OprNode* hold_node        );
        virtual void               add_to_cycle_det       (NodeWrapCycleDet& deter  ) const = 0;
        virtual void               assign_depend_dent    (SequenceEle* predecessor ) const = 0;
        virtual void               assign_int_start      (OprNode* int_start_node    ) = 0;
        //////// get/check command
                OprNode*           get_int_reset_node     () const {return _intRstNode;}
                OprNode*           get_hold_node_ptr         () const {return _holdNode;}
        virtual Node*              get_state_finish_iden  () const = 0;
        virtual std::vector<Node*> get_entrance_nodes    () = 0;
        virtual bool               is_there_force_exit_node() const = 0;
        virtual Node*              get_force_exit_node_ptr    () const = 0;
        virtual bool               is_node_wrap          () const = 0;
        virtual NodeWrap*          get_node_wrap         () const = 0;
        virtual bool               is_basic_node         () const = 0;
        virtual StateNode*         get_basic_node        () const = 0;
        virtual std::string        get_describe         () = 0;
        virtual void               add_to_system_nodes    (std::vector<Node*>& sys_node) = 0;

        virtual void               assign_depend_dent    (Node* activator_node) const = 0;

    };

    /**
     * the basic element wrapper
     */
    class SequenceEleBasic: public SequenceEle{
        AsmNode*   _asmNode    = nullptr;
        StateNode* _stateNode  = nullptr;
    public:
        explicit SequenceEleBasic(Node* asm_node);
        ~SequenceEleBasic() override;

        void               gen_node         (CLOCK_MODE cm)                       override;
        void               set_ident_state_id (ull master_idx, int sub_idx) const     override;
        void               add_to_cycle_det       (NodeWrapCycleDet& deter) const   override;
        void               assign_depend_dent    (SequenceEle* predecessor) const  override;
        void               assign_int_start      (OprNode* int_start_node)           override;
        Node*              get_state_finish_iden  () const                          override;
        std::vector<Node*> get_entrance_nodes    ()                                override;
        bool               is_there_force_exit_node() const                          override;
        Node*              get_force_exit_node_ptr    () const                          override;
        bool               is_node_wrap          () const                          override;
        NodeWrap*          get_node_wrap         () const                          override;
        bool               is_basic_node         () const                          override;
        StateNode*         get_basic_node        () const                          override;
        std::string        get_describe         ()                                override;
        void               add_to_system_nodes    (std::vector<Node*>& sys_node)     override;

        /////// extension for custom dependency
        void               assign_depend_dent    (Node* activator_node) const override;

    };

    /**
     * the flow block wrapper
     */
    class SequenceEleFlowBlock: public SequenceEle{
        FlowBlockBase* _subBlock    = nullptr;
        NodeWrap*      _complexNode = nullptr;
    public:
        explicit SequenceEleFlowBlock(FlowBlockBase* fb_base);

        void               gen_node         (CLOCK_MODE cm)                       override;
        void               set_ident_state_id (ull master_idx, int sub_idx) const     override;
        void               add_to_cycle_det       (NodeWrapCycleDet& deter) const   override;
        void               assign_depend_dent    (SequenceEle* predecessor) const  override;
        void               assign_int_start      (OprNode* int_start_node)           override;
        Node*              get_state_finish_iden  () const                          override;
        std::vector<Node*> get_entrance_nodes    ()                                override;
        bool               is_there_force_exit_node() const                          override;
        Node*              get_force_exit_node_ptr    () const                          override;
        bool               is_node_wrap          () const                          override;
        NodeWrap*          get_node_wrap         () const                          override;
        bool               is_basic_node         () const                          override;
        StateNode*         get_basic_node        () const                          override;
        std::string        get_describe         ()                                override;
        void               add_to_system_nodes    (std::vector<Node*>& sys_node)     override;

        /////// extension for custom dependency
        void               assign_depend_dent    (Node* activator_node) const override;

    };

}

#endif //MODEL_FLOWBLOCK_SEQ_SEQELE_H