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

        explicit PseudoNode(int expr_size, LOGIC_OP joinOp) :
                Node(PSEUDO_NODE),
                _pseudoAssignMeta(new expression(expr_size)),
                _joinOp(joinOp){

            set_clock_mode(CM_CLK_FREE);

        }

        void assign() override{
            assert(!_node_srcs.empty());
            Operable* finalOpr  = nullptr;
            Operable* oprPerSrc = nullptr;
            for (auto nodeSrc: _node_srcs){
                assert(nodeSrc.dependNode != nullptr);
                oprPerSrc = nodeSrc.dependNode->get_exit_opr_ptr();
                if (nodeSrc.condition != nullptr){
                    assert(nodeSrc.condition->getOperableSlice().getSize() == 1);
                    add_logic(oprPerSrc, nodeSrc.condition, BITWISE_AND);
                }
                assert(oprPerSrc != nullptr);
                add_logic(finalOpr, oprPerSrc, _joinOp);
            }

            assert(finalOpr != nullptr);
            *_pseudoAssignMeta = (*finalOpr);
            assert(_pseudoAssignMeta != nullptr);
            _pseudoAssignMeta->setVarName(_ident_name);
        }
        int get_cycle_used() override { return 0; }

        Operable* get_exit_opr_ptr() override{return _pseudoAssignMeta;}

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
            assert(_node_srcs.empty());
            _value->setVarName(_ident_name);
        }

        int get_cycle_used() override{ return 0; }

        Operable* get_exit_opr_ptr() override{return _value;}

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
            assert(_node_srcs.empty());
            ////_value->setVarName(identName);
        }

        int get_cycle_used() override{ return 0; }

        Operable* get_exit_opr_ptr() override{return _value;}

        bool is_state_full_node() override{return false;}

    };

}

#endif //KATHRYN_LOGICNODE_H
