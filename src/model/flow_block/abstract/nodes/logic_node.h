//
// Created by tanawin on 15/4/2567.
//

#ifndef KATHRYN_LOGICNODE_H
#define KATHRYN_LOGICNODE_H

#include "node.h"


namespace kathryn{

    struct PseudoNode : Node{
        expression* _pseudoAssignMeta = nullptr;
        LOGIC_OP    _joinOp;

        explicit PseudoNode(int expr_size, LOGIC_OP join_op) :
                Node(PSEUDO_NODE),
                _pseudoAssignMeta(new expression(expr_size)),
                _joinOp(join_op){

            set_clock_mode(CM_CLK_FREE);

        }

        void assign() override{
            assert(!node_srcs.empty());
            Operable* final_opr  = nullptr;
            Operable* opr_per_src = nullptr;
            for (auto node_src: node_srcs){
                assert(node_src.depend_node != nullptr);
                opr_per_src = node_src.depend_node->get_exit_opr_ptr();
                if (node_src.condition != nullptr){
                    assert(node_src.condition->get_operable_slice().get_size() == 1);
                    add_logic(opr_per_src, node_src.condition, BITWISE_AND);
                }
                assert(opr_per_src != nullptr);
                add_logic(final_opr, opr_per_src, _joinOp);
            }

            assert(final_opr != nullptr);
            *_pseudoAssignMeta = (*final_opr);
            assert(_pseudoAssignMeta != nullptr);
            _pseudoAssignMeta->set_var_name(ident_name);
        }
        int get_cycle_used() override { return 0; }

        Operable*get_exit_opr_ptr() override{return _pseudoAssignMeta;}

        bool is_state_full_node() override{ return false; }

    };

    struct DummyNode : Node{
        Val* _value = nullptr;

        explicit DummyNode(Val* value) :
                Node(DUMMY_NODE),
                _value(value){
            assert(_value != nullptr);
        }

        void assign() override{
            /** we don't support assign from condition or depend state*/
            assert(node_srcs.empty());
            _value->set_var_name(ident_name);
        }

        int get_cycle_used() override{ return 0; }

        Operable*get_exit_opr_ptr() override{return _value;}

        bool is_state_full_node() override{return false;}

    };

    struct OprNode : Node{
        Operable* _value = nullptr;

        explicit OprNode(Operable* value) :
                Node(OPR_NODE),
                _value(value){
            assert(_value != nullptr);
            set_clock_mode(CM_CLK_FREE);
        }

        void assign() override{
            /** we don't support assign from condition or depend state*/
            assert(node_srcs.empty());
            ////_value->set_var_name(ident_name);
        }

        int get_cycle_used() override{ return 0; }

        Operable*get_exit_opr_ptr() override{return _value;}

        bool is_state_full_node() override{return false;}

    };

}

#endif //KATHRYN_LOGICNODE_H
