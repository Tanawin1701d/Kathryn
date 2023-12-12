//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_NODE_H
#define KATHRYN_NODE_H

#include "model/hwComponent/abstract/operation.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/abstract/assignable.h"
#include<vector>
#include<memory>

namespace kathryn {


/** atomic node for assigning */
    struct Node {

        bool pseudoNode = false;
        Operable *psudoAssignMeta = nullptr;
        AssignMeta *assignMeta = nullptr; //// AssignMeta is must not use the same assign metas
        Operable *condition = nullptr;
        Operable *dependState = nullptr;

        /** This constructor is use for psuedo Node */
        Node() : pseudoNode(true), assignMeta(nullptr),
                 psudoAssignMeta(new expression()) {}

        Node(Node &rhs) { operator=(rhs); }

        explicit Node(AssignMeta *asmMeta) : assignMeta(asmMeta) {}

        ~Node() { delete assignMeta; }

        Node &operator=(const Node &rhs) {
            if (&rhs == this)
                return *this;
            pseudoNode = rhs.pseudoNode;
            psudoAssignMeta = rhs.psudoAssignMeta;
            assignMeta = new AssignMeta(*rhs.assignMeta);
            condition = rhs.condition;
            dependState = rhs.dependState;
            return *this;
        }

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
        void addCondtion(Operable *opr, LOGIC_OP op) {
            addLogic(condition, opr, op);
        }

        /** add dependState for assignment*/
        void addDependState(Operable *opr, LOGIC_OP op) {
            addLogic(dependState, opr, op);
        }

        /** assign data to register*/
        void assign() {
            if (pseudoNode) {
                (*psudoAssignMeta) = *(Operable *) (&(*dependState & *condition));
            } else {
                auto resultUpEvent = new UpdateEvent({
                                                             condition,
                                                             dependState,
                                                             &assignMeta->valueToAssign,
                                                             assignMeta->desSlice,
                                                             9
                                                     });

                assignMeta->updateEventsPool.push_back(resultUpEvent);
            }
        }

        bool isPseudoNode() const{return pseudoNode;}

    };

    /* use for inner block return to outter block**/
    struct NodeWrap {
    public:
        static const int IN_CONSIST_CYCLE_USED = -1;
        /** entrance represent UpdateEvent which refers to node that be head of the subblock*/
        /** note that exprMetas must not be here due to the abstract of the system*/
        std::vector<Node *> entranceNodes;
        /** the exit condition that allow next building block run*/
        Operable *exitOpr = nullptr;
        /** number of cycle required in this subblock*/
        int cycleUsed = IN_CONSIST_CYCLE_USED;

        NodeWrap(const NodeWrap &rhs) {
            exitOpr = nullptr;
            *this = rhs;
        }

        NodeWrap() = default;

        NodeWrap &operator=(const NodeWrap &rhs) {
            if (&rhs == this) {
                return *this;
            }
            ///// change node location
            for (auto nd: rhs.entranceNodes) {
                Node *preInsert = new Node(*nd);
                entranceNodes.push_back(preInsert);
            }
            exitOpr = rhs.exitOpr;
            return *this;
        }

        void addEntraceNode(Node *nd) {
            assert(nd != nullptr);
            entranceNodes.push_back(nd);
        }

        void addExitOpr(Operable *opr) {
            assert(opr != nullptr);
            exitOpr = opr;
        }

        void addConditionToAllNode(Operable *cond, LOGIC_OP op) {
            assert(cond != nullptr);
            for (auto node: entranceNodes) {
                node->addCondtion(cond, op);
            }
        }

        void addDependStateToAllNode(Operable *st, LOGIC_OP op) {
            assert(st != nullptr);
            for (auto node: entranceNodes) {
                node->addDependState(st, op);
            }
        }

        void assignAllNode() {
            for (auto node: entranceNodes) {
                node->assign();
            }
        }

        /** copy node pointer to this wrap*/
        /// todo we will make it copy node if need but for now we don't
        void transferNodeFrom(NodeWrap *nw) {
            assert(nw != nullptr);
            for (auto node: nw->entranceNodes) {
                entranceNodes.push_back(node);
            }
        }

        void deleteNodesInWrap() {
            for (auto nd: entranceNodes) {
                delete nd;
            }
        }

        void setCycleUsed(int cycle){
            assert(cycle == -1 || cycle > 0);
            cycleUsed = cycle;
        }

    };

    /** this struct is used to determine numbers of cycle
     * that is used in multiple subblock
     * if return -1 means can determine exact value
     * else x > 0 means these sub flow block use x cycle to
     * complete their work equally
     * */

    struct NodeWrapCycleDet{
        std::vector<int> samplingVec;

        void addToDet(std::vector<Node*>& nodes){
            for (auto nd: nodes){
                addToDet(nd);
            }
        }

        void addToDet(std::vector<NodeWrap*>& nws){
            for (auto nw: nws){
                addToDet(nw);
            }
        }

        void addToDet(Node* nd){
            assert(nd != nullptr);
            if (nd->isPseudoNode()) {
                samplingVec.push_back(0);
            }else{
                samplingVec.push_back(1);
            }
        }
        void addToDet(NodeWrap* nw){
            assert(nw != nullptr);
            samplingVec.push_back(nw->cycleUsed);
        }

        int getCycleHorizon(){
            assert(!samplingVec.empty());
            int testVal = samplingVec[0];

            for (auto cycle: samplingVec){
                /**case detect in consistent in sub block*/
                if (cycle == NodeWrap::IN_CONSIST_CYCLE_USED){
                    return NodeWrap::IN_CONSIST_CYCLE_USED;
                }
                /** check that it is equal to other*/
                if (testVal != cycle){
                    return NodeWrap::IN_CONSIST_CYCLE_USED;
                }
            }
            assert(testVal > 0);
            /** return only when sampling is all equal*/
            return testVal;
        }

        int getCycleVertical(){
            assert()
        }

    };
}

#endif //KATHRYN_NODE_H
