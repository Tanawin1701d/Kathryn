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
                _joinOp(joinOp)
        {}

        void assign() override{
            assert(!nodeSrcs.empty());
            Operable* finalOpr = nullptr;
            Operable* oprPerSrc = nullptr;
            for (auto nodeSrc: nodeSrcs){
                oprPerSrc = nodeSrc.dependNode->getExitOpr();
                if (nodeSrc.condition == nullptr){
                    assert(nodeSrc.condition->getOperableSlice().getSize() == 1);
                    addLogic(oprPerSrc, nodeSrc.condition, BITWISE_AND);
                }
                assert(oprPerSrc != nullptr);
                addLogic(finalOpr, oprPerSrc, _joinOp);
            }

            assert(finalOpr != nullptr);
            *_pseudoAssignMeta = *finalOpr;
            assert(_pseudoAssignMeta != nullptr);
            _pseudoAssignMeta->setVarName(identName);
        }
        int getCycleUsed() override { return 0; }

        Operable* getExitOpr() override{return _pseudoAssignMeta;}

        bool isStateFullNode() override{ return false; }

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_pseudoAssignMeta != nullptr);
            /** inc engine do not increase engine*/

        }



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
            assert(nodeSrcs.empty());
            _value->setVarName(identName);
        }

        int getCycleUsed() override{ return 0; }

        Operable* getExitOpr() override{return _value;}

        bool isStateFullNode() override{return false;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_value != nullptr);
            /** inc engine do not increase engine*/
        }

    };

    struct OprNode : Node{
        Operable* _value = nullptr;

        explicit OprNode(Operable* value) :
                Node(OPR_NODE),
                _value(value){
            assert(_value != nullptr);
        }

        void assign() override{
            /** we don't support assign from condition or depend state*/
            assert(nodeSrcs.empty());
            ////_value->setVarName(identName);
        }

        int getCycleUsed() override{ return 0; }

        Operable* getExitOpr() override{return _value;}

        bool isStateFullNode() override{return false;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_value != nullptr);
            /** inc engine do not increase engine*/
        }

    };

}

#endif //KATHRYN_LOGICNODE_H
