//
// Created by tanawin on 31/12/2566.
//

#ifndef KATHRYN_NODEWRAP_H
#define KATHRYN_NODEWRAP_H

#include "model/FlowBlock/abstract/nodes/node.h"

namespace kathryn{

    static const int IN_CONSIST_CYCLE_USED = -1;

    /* use for inner block return to outter block**/
    struct NodeWrap {
    public:

        /** entrance represent UpdateEvent which refers to node that be head of the subblock*/
        /** note that exprMetas must not be here due to the abstract of the system*/
        std::vector<Node*> entranceNodes;
        /** the exit condition that allow next building block run*/
        Node* exitNode  = nullptr;
        /** number of cycle required in this subblock*/
        int cycleUsed = IN_CONSIST_CYCLE_USED;

        NodeWrap(const NodeWrap& rhs) {
            *this = rhs;
        }

        NodeWrap() = default;

        NodeWrap &operator=(const NodeWrap& rhs) {
            if (&rhs == this) {
                return *this;
            }
            ///// change node location
            for (auto nd: rhs.entranceNodes) {
                assert(nd != nullptr);
                entranceNodes.push_back(nd->clone());
            }
            /** exit expression is not copied*/
            exitNode  = rhs.exitNode;
            return *this;
        }

        void addEntraceNode(Node *nd) {
            assert(nd != nullptr);
            entranceNodes.push_back(nd);
        }

        void addEntraceNodes(std::vector<Node*> nds){
            for (auto nd : nds){
                assert(nd != nullptr);
                entranceNodes.push_back(nd);
            }
        }

        void addExitNode(Node* nd) {
            assert(nd != nullptr);
            exitNode = nd;
        }

        void addConditionToAllNode(Operable *cond, LOGIC_OP op) {
            assert(cond != nullptr);
            for (auto node: entranceNodes) {
                node->addCondtion(cond, op);
            }
        }

        void addDependNodeToAllNode(Node* st) {
            assert(st != nullptr);
            for (auto node: entranceNodes) {
                node->addDependNode(st);
            }
        }

        void setDependNodeCond(LOGIC_OP op){
            for (auto node: entranceNodes){
                node->setDependStateJoinOp(op);
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

        Node* getExitNode () const { return exitNode; }

        void setCycleUsed(int cycle){
            assert(cycle == -1 || cycle > 0);
            cycleUsed = cycle;
        }

        int getCycleUsed() const{
            assert(cycleUsed == -1 || cycleUsed > 0 );
            return cycleUsed;
        }

    };

    /** this struct is used to determine numbers of cycle
     * that is used in multiple subblock
     * if return -1 means can't determine exact value
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
            samplingVec.push_back(nd->getCycleUsed());
        }
        void addToDet(NodeWrap* nw){
            assert(nw != nullptr);
            samplingVec.push_back(nw->getCycleUsed());
        }

        int getMaxCycleHorizon(){
            assert(!samplingVec.empty());
            int testVal = samplingVec[0];

            for (auto cycle: samplingVec){
                /**case detect in consistent in sub block*/
                if (cycle == IN_CONSIST_CYCLE_USED){
                    return IN_CONSIST_CYCLE_USED;
                }
                /** check that it is equal to other*/

                testVal = std::max(testVal, cycle);
            }
            assert(testVal >= 0);
            /** return only when sampling is all equal*/
            return testVal;
        }

        int getSameCycleHorizon(){
            assert(!samplingVec.empty());
            int testVal = samplingVec[0];
            for (auto cycle: samplingVec){
                if (cycle == IN_CONSIST_CYCLE_USED){
                    return IN_CONSIST_CYCLE_USED;
                }
                if (testVal != cycle){
                    return IN_CONSIST_CYCLE_USED;
                }
            }
            return testVal;
        }

        int getCycleVertical(){
            assert(!samplingVec.empty());
            int cycleUsed = 0;
            for (auto subCycle: samplingVec){
                if (subCycle == IN_CONSIST_CYCLE_USED){
                    return IN_CONSIST_CYCLE_USED;
                }
                cycleUsed += subCycle;

            }
            return cycleUsed;
        }

        /** search for node that have match to input cycle*/
        static Node* getMatchNode(const std::vector<Node*>& nds, int cycle){
            assert(cycle != IN_CONSIST_CYCLE_USED);
            ////// we do not allow in consist cycle to be matched with node
            for (auto nd : nds){
                if (nd == nullptr){
                    continue;
                }
                if (nd->getCycleUsed() == cycle){
                    return nd;
                }
            }
            return nullptr;
        }

        /** search for node wrap that have match to input cycle*/
        static NodeWrap* getMatchNodeWrap(const std::vector<NodeWrap*>& nws, int cycle){
            assert(cycle != IN_CONSIST_CYCLE_USED);
            for (auto nw: nws){
                if (nw == nullptr){
                    continue;
                }
                if (cycle == nw->getCycleUsed()){
                    return nw;
                }
            }
            return nullptr;
        }

    };

    /** get the exit operable of matched node/nodeWrap that got same cycle used*/
    static Node* getMatchExitExpr(const std::vector<Node*>& nds,
                                      const std::vector<NodeWrap*>& nws,
                                      int cycle
    ){

        assert(cycle >= 0);

        Node* matchedNode = NodeWrapCycleDet::getMatchNode(nds, cycle);

        if (matchedNode != nullptr){
            return matchedNode;
        }
        NodeWrap* matchedNodeWrap = NodeWrapCycleDet::getMatchNodeWrap(nws, cycle);

        if (matchedNodeWrap != nullptr){
            return matchedNodeWrap->getExitNode();
        }

        return nullptr;

    }


}

#endif //KATHRYN_NODEWRAP_H
