//
// Created by tanawin on 31/12/2566.
//

#ifndef KATHRYN_NODEWRAP_H
#define KATHRYN_NODEWRAP_H

#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/nodeCon.h"

namespace kathryn{

    static const int IN_CONSIST_CYCLE_USED = -1;

    /* use for inner block return to outter block**/
    struct NodeWrap : ModelDebuggable {
    public:

        /** it is agent of the node typically ctrlFlowRegBase*/
        std::vector<SrcNodeAgent> srcNodeAgents;
        /** the exit condition that allow next building block run*/
        Node* exitNode      = nullptr;
        /** force exit Node is the node that indicate exit expression without concerning of flowBlock behaviour*/
        Node* forceExitNode = nullptr;
        /** bypass condition may be there is no state to handle in previous wrap*/
        Operable* byPassCondition = nullptr;
        /** number of cycle required in this subblock*/
        int cycleUsed       = IN_CONSIST_CYCLE_USED;
        /** disable copy constructor*/
//        NodeWrap(const NodeWrap& rhs) = delete;
//        NodeWrap& operator = (const NodeWrap& rhs) = delete;

        NodeWrap() = default;

        /**
         * add/set element of node wrap
         * */

        void addEntraceNode(SrcNodeAgent sna) {
            srcNodeAgents.push_back(sna);
        }

        void addEntraceNodes(const std::vector<SrcNodeAgent>& nds){
            for (auto nd : nds){
                addEntraceNode(nd);
            }
        }

        void setExitNode(Node* nd) {
            assert((nd != nullptr) && (exitNode == nullptr));
            exitNode = nd;
        }

        void setForceExitNode(Node* nd){
            assert((nd != nullptr) && (forceExitNode == nullptr));
            forceExitNode = nd;
        }

        void setByPassCond(Operable* bypassOpr){
            assert((bypassOpr != nullptr) && (byPassCondition == nullptr));
        }

        /** add dep and condition condition*/

        void addDepToAllSrcAgent(Node* masterSrcNode, CONNECT_NODE_PURPOSE cnp){
            assert(masterSrcNode != nullptr);
            assert(cnp < CON_NODE_CNT);
            for (auto srcNodeAgent: srcNodeAgents) {
                srcNodeAgent.addDep(masterSrcNode, cnp);
            }
        }

        void addCondToAllSrcAgent(Operable *cond, LOGIC_OP op) {
            assert(cond != nullptr);
            for (auto sng: srcNodeAgents) {
                sng.addCond(cond, op);
            }
        }

        void addCondToByPass(Operable* cond, LOGIC_OP op) {
            assert(cond != nullptr);
            addLogic(byPassCondition, cond, op);
        }

        /**
         *
         * finalize dep
         *
         * */

        void finalizeAllSrcNodeAgent() {
            for (auto srcNodeAgent: srcNodeAgents) {
                srcNodeAgent.finalize();
            }
        }

        /** copy node pointer to this wrap*/
        /// todo we will make it copy node if need but for now we don't
        void transferEntSrcNodeAgentFrom(NodeWrap *nw) {
            assert(nw != nullptr);
            for (auto node: nw->srcNodeAgents) {
                srcNodeAgents.push_back(node);
            }
        }

        Operable* getBypassCond() const{
            return byPassCondition;
        }

        Node* getExitNode () const {
            assert(exitNode != nullptr);
            return exitNode;
        }
        bool  isThereForceExitNode() const {return forceExitNode != nullptr;}
        Node* getForceExitNode    () const {return forceExitNode;}

        void setCycleUsed(int cycle){
            assert(cycle == -1 || cycle > 0);
            cycleUsed = cycle;
        }

        int getCycleUsed() const{
            assert(cycleUsed == -1 || cycleUsed > 0 );
            return cycleUsed;
        }

        std::string getMdIdentVal() override{
            return "[nodeWrap @" + std::to_string((ull)this) + " ]";
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

        /** search for node that have match to input cycle skip if node is nullptr*/
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

        /** search for node wrap that have match to input cycle skip if node is nullptr*/
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

    /** get the exit node of matched node that got same cycle used*/
    static Node* getMatchNodeFromNdsOrNws(const std::vector<Node*>& nds,
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

    /** get the any exit node that first*/
    static Node* getAnyNodeFromNdsOrNws(const std::vector<Node*>& nds,
                                        const std::vector<NodeWrap*>& nws){
        for (auto nd : nds){
            if (nd != nullptr){
                return nd;
            }
        }
        for (auto nw : nws){
            if (nw != nullptr){
                return nw->getExitNode();
            }
        }
        return nullptr;
    }

}

#endif //KATHRYN_NODEWRAP_H
