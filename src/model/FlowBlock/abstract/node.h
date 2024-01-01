//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_NODE_H
#define KATHRYN_NODE_H

#include "model/hwComponent/abstract/operation.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/abstract/assignable.h"
#include "stateReg.h"
#include<vector>
#include<memory>
#include <queue>

namespace kathryn {


    struct NodeWrap;


/** atomic node for assigning */
    struct Node {

        Operable* condition = nullptr;
        std::vector<Node*> dependNodes;
        LOGIC_OP dependStateRaiseCond;

        Node(Node &rhs) {
            condition = rhs.condition;
            dependNodes = rhs.dependNodes;
            dependStateRaiseCond = rhs.dependStateRaiseCond;
        }

        explicit Node() = default;

        virtual ~Node() = default;

        virtual Node* clone() = 0;

        static void addLogic(Operable *&desLogic, Operable *opr, LOGIC_OP op) {
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

        /** add condition for assignment*/
        virtual void addCondtion(Operable* opr, LOGIC_OP op) {
            addLogic(condition, opr, op);
        }

        /** add dependNode for assignment*/
        void addDependNode(Node* srcNode) {
            assert(srcNode != nullptr);
            dependNodes.push_back(srcNode);
        }

        std::vector<Node*>& getDependNodes() {return dependNodes; }

        /** join depend node to usealble expression*/
        Operable* getAllDependNodeOpr(){
            Operable* resultOpr = nullptr;
            for (auto nd : dependNodes){
                addLogic(resultOpr, nd->getExitOpr(), dependStateRaiseCond);
            }
            assert(resultOpr != nullptr);
            return resultOpr;
        }
        /** set that how to join node to make this node valid*/
        virtual void setDependStateJoinOp(LOGIC_OP op){
            dependStateRaiseCond = op;
        }
        /** provided src state data*/
        virtual Operable* getExitOpr(){ return nullptr; };
        /** assign value*/
        virtual void assign() = 0;
        /** cycle that is use in this node*/
        virtual int getCycleUsed() = 0;
        /** is Stateful node (reffer to node that consume at least 1 cycle from machine)*/
        virtual bool isStateFullNode(){ return true; }

    };

    struct AsmNode : Node{
        AssignMeta* _assignMeta = nullptr; //// AssignMeta is must not use the same assign metas

        explicit AsmNode(AssignMeta* assignMeta) :
            Node(),
            _assignMeta(assignMeta){
            assert(_assignMeta != nullptr);
        }

        Node* clone() override{
            auto clNode = new AsmNode(*this);
            return clNode;
        }

        void assign() override{
            auto resultUpEvent = new UpdateEvent({
                                                         condition,
                                                         getAllDependNodeOpr(),
                                                         &_assignMeta->valueToAssign,
                                                         _assignMeta->desSlice,
                                                         9
                                                 });
            _assignMeta->updateEventsPool.push_back(resultUpEvent);
        }

        int getCycleUsed() override { return 1; }

    };

    /**
     * state that represent status of each register in the circuit
     * */

    struct StateNode : Node{
        StateReg* _stateReg;

        explicit StateNode() :
            Node(),
            _stateReg(new StateReg()){}

        Node* clone() override{
            auto clNode = new StateNode(*this);
            return clNode;
        }

        Operable* getExitOpr() override{
            assert(_stateReg != nullptr);
            return _stateReg->generateEndExpr();
        }

        void assign() override{
            auto dependNodeOpr = getAllDependNodeOpr();
            assert(dependNodeOpr != nullptr);
            _stateReg->addUpdateEvent(dependNodeOpr);
        }

        int getCycleUsed() override {return 1;}
    };

    /**
     * node that represent syn status of each register in the circuit
     * usually used in parallel block with unknown exact cycle
     * */

    struct SynNode : Node{
        StateReg* _synReg;

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int synSize) : _synReg(new StateReg(synSize)){}

        Node* clone() override{
            /** syn node is not supposed to be copy*/
            assert(false);
        }

        void addCondtion(Operable* opr, LOGIC_OP op) override{ assert(false);}

        void addDependState(NodeWrap* srcNw, LOGIC_OP op);

        Operable* getExitOpr() override{return _synReg->generateEndExpr();}

        void assign() override{
            for (size_t i = 0; i < dependNodes.size(); i++){
                _synReg->addUpdateEvent(dependNodes[i]->getExitOpr()
                                        , (int)i);
            }
        }

        int getCycleUsed() override{ return 1; }

    };

    struct PseudoNode : Node{
        expression* _pseudoAssignMeta = nullptr;

        explicit PseudoNode() :
            Node(),
            _pseudoAssignMeta(new expression()){}

        Node* clone() override{
            auto clNode = new PseudoNode(*this);
            return clNode;
        }

        void assign() override{
            _pseudoAssignMeta = &(*getAllDependNodeOpr() & *condition);
        }
        int getCycleUsed() override { return 0; };

        bool isStateFullNode() override{ return false; }

    };

    /** This function is used to checked that start node is hard wired to some node in
     * nds if it is match return true
     *
     * please bear in mind that copy node that used in while loop can't be detect by this
     * function
     * */
    bool thereAreStateLessConnection(std::vector<Node*> nds, Node* startNode){
        assert(!startNode->isStateFullNode());
        std::queue<Node*> toCheckingNode;

        /**fill checking node from nds*/
        toCheckingNode.push(startNode);

        while (!toCheckingNode.empty()){
            auto frontNode = toCheckingNode.front();
            toCheckingNode.pop();
            /** check depend matched node*/
            for (auto checkNode: nds){
                if (checkNode == frontNode){
                    return true;
                }
            }
            /**add dependNode to next iteration*/
            for (auto depNode : frontNode->getDependNodes()){
                assert(depNode != nullptr);
                if (!depNode->isStateFullNode()){
                    toCheckingNode.push(depNode);
                }
            }

        }

        return false;


    }

}

#endif //KATHRYN_NODE_H
