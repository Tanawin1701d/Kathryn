//
// Created by tanawin on 8/1/2567.
//
#include"node.h"

namespace kathryn{


    std::string NT_to_string(NODE_TYPE nt){

        std::string mapper[NODE_TYPE_CNT] = {
        "ASM_NODE",
        "STATE_NODE",
        "SYN_NODE",
        "PSEUDO_NODE",
        "DUMMY_NODE",
        "START_NODE",
        "WAITCOND_NODE",
        "WAITCYCLE_NODE",
        "INTERRUPT_NODE"
        };
        return mapper[nt];

    }

    /** node base*/

    SrcNodeAgent Node::genSrcAgent() {
        return  {nullptr, this};
    }


    /** src node agent*/

    SrcNodeAgent::SrcNodeAgent(Operable* cond, Node* desNode):
    _condition(cond),
    _desNode(desNode){
        assert(_desNode != nullptr);
    }

    void SrcNodeAgent::addDep(Node* srcNode, CONNECT_NODE_PURPOSE cnp) const{
        assert(srcNode != nullptr);
        _desNode->allocDep(cnp, _condition, srcNode);
    }

    void SrcNodeAgent::addCond(Operable* cond, LOGIC_OP op){
        assert(cond != nullptr);
        addLogic(_condition, cond, op);
    }

    void SrcNodeAgent::finalize() const{
        _desNode->finalize();
    }







    void addLogic(Operable* &desLogic, Operable *opr, LOGIC_OP op){
        assert(op == BITWISE_AND || op == BITWISE_OR);
        assert(opr != nullptr);
        if (desLogic == nullptr) {
            desLogic = opr;
            return;
        }

        if (op == BITWISE_AND) {
            desLogic = &((*desLogic) & (*opr));
        } else if (op == BITWISE_OR) {
            desLogic = &((*desLogic) | (*opr));
        }
    }



}