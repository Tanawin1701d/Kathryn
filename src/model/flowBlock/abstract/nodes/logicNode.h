//
// Created by tanawin on 15/4/2567.
//

#ifndef KATHRYN_LOGICNODE_H
#define KATHRYN_LOGICNODE_H

#include "node.h"


namespace kathryn{

    struct PseudoNode : Node{
        expression* _pseudo_assign_meta = nullptr;
        LOGIC_OP    _join_op;

        explicit PseudoNode(int expr_size, LOGIC_OP joinOp) :
                Node(PSEUDO_NODE),
                _pseudo_assign_meta(new expression(expr_size)),
                _join_op(joinOp){

            set_clock_mode(CM_CLK_FREE);

        }

        void assign() override{
            assert(!_node_srcs.empty());
            Operable* final_opr  = nullptr;
            Operable* opr_per_src = nullptr;
            for (auto node_src: _node_srcs){
                assert(node_src.depend_node != nullptr);
                opr_per_src = node_src.depend_node->get_exit_opr_ptr();
                if (node_src.condition != nullptr){
                    assert(node_src.condition->getOperableSlice().getSize() == 1);
                    add_logic(opr_per_src, node_src.condition, BITWISE_AND);
                }
                assert(opr_per_src != nullptr);
                add_logic(final_opr, opr_per_src, _join_op);
            }

            assert(final_opr != nullptr);
            *_pseudo_assign_meta = (*final_opr);
            assert(_pseudo_assign_meta != nullptr);
            _pseudo_assign_meta->setVarName(_ident_name);
        }
        int       get_cycle_used() override { return 0; }

        Operable* get_exit_opr_ptr() override{return _pseudo_assign_meta;}

        bool      is_stateful_node() override{ return false; }

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
            assert(_node_srcs.empty());
            _value->setVarName(_ident_name);
        }

        int       get_cycle_used() override{ return 0; }

        Operable* get_exit_opr_ptr() override{return _value;}

        bool      is_stateful_node() override{return false;}

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
            assert(_node_srcs.empty());
            ////_value->setVarName(identName);
        }

        int       get_cycle_used() override{ return 0; }

        Operable* get_exit_opr_ptr() override{return _value;}

        bool      is_stateful_node() override{return false;}

    };

}

#endif //KATHRYN_LOGICNODE_H
