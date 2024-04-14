//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_NODE_H
#define KATHRYN_NODE_H

#include <utility>
#include<vector>
#include<memory>
#include<queue>
#include<map>

#include "model/hwComponent/abstract/operation.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/spReg/syncReg.h"
#include "model/debugger/modelDebugger.h"
#include "model/simIntf/flowBlock/flowBlockSimEngine.h"


namespace kathryn {

    struct NodeWrap;

    enum NODE_TYPE{
        ASM_NODE,
        STATE_NODE,
        SYN_NODE,
        PSEUDO_NODE,
        DUMMY_NODE,
        START_NODE,
        WAITCOND_NODE,
        WAITCYCLE_NODE,
        NODE_TYPE_CNT
    };


    /***
     * NODE operation requirement
     * 1. declare Node
     * 2. add condition (optional)
     * 3. add dependNode (at least one node)
     * 4. set joinDependNodeOp (required)
     * 5. assign (required)
     * 1-4 can be set independently order, but 5 must declare at last time.
     * **/

    std::string NT_to_string(NODE_TYPE nt);

    struct NodeSrcEdge{
        Node*     dependNode = nullptr;
        Operable* condition  = nullptr;
    };

    struct Node : public ModelDebuggable,
                  public FlowSimEngine{
        NODE_TYPE                nodeType  = NODE_TYPE_CNT;
        std::vector<NodeSrcEdge> nodeSrcs;
        std::string              identName = "NODE_UNNAME";

        Operable*                intReset  = nullptr;

        Node(Node& rhs) = delete;

        explicit Node(NODE_TYPE nt):
                FlowSimEngine(),
                nodeType(nt){};

        ~Node() override = default;

        NODE_TYPE getNodeType() const{
            return nodeType;
        }

        static void addLogic(Operable* &desLogic, Operable *opr, LOGIC_OP op) {
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

        /** add dependNode for assignment*/
        void addDependNode(Node* srcNode, Operable* cond) {
            assert(srcNode != nullptr);
            nodeSrcs.push_back({srcNode, cond});
        }

        std::vector<NodeSrcEdge>& getDependNodes() {return nodeSrcs;}
        void setInterruptReset(Operable* rst){
            assert(rst != nullptr);
            assert(rst->getOperableSlice().getSize() == 1);
            intReset = rst;
        }
        Operable* getInterruptReset(){
            return intReset;
        }
        /**
         * function that allow sp node custom their behavior
         * **/
        /** unset event when state is raised there must be condition that bring this down*/
        virtual void      makeUnsetStateEvent(){assert(false);}
        /** provided src state data*/
        virtual Operable* getExitOpr(){ return nullptr; };
        /** assign value with proper condition*/
        virtual void      assign() = 0; /** please make sure that makeunsetState is called*/
        virtual void      dryAssign(){assert(false);};
        /** cycle that is use in this node*/
        virtual int       getCycleUsed() = 0;
        /** is Stateful node (reffer to node that consume at least 1 cycle from machine)*/
        virtual bool      isStateFullNode(){ return true; }



        /** get debugger value*/


        std::string getMdIdentVal() override{
            std::string ret = NT_to_string(nodeType) + " @ " + std::to_string((ull)this);
            return ret;
        }

        void addMdLog(MdLogVal* mdLogVal) override{
            mdLogVal->addVal("[Node] " + getMdIdentVal() +  "have node dep");
            for (auto depSrc : nodeSrcs){
                mdLogVal->addVal(depSrc.dependNode->getMdIdentVal());
            }
        }

        /*** simulation override*/

        void simExitCurCycle() override{
            unsetBlockOrNodeRunning();
            unSetSimStatus();
        }

        /** internal value identifier for debugging purpose*/
        void setInternalIdent(std::string identVal){
            identName = std::move(identVal);
        }



    };

    /** This function is used to checked that start node is hard wired to some node in
     * nds if it is match return true
     *
     * please bear in mind that copy node that used in while loop can't be detect by this
     * function
     * */
    bool thereAreStateLessConnection(std::vector<Node*> nds, Node* startNode);

}

#endif //KATHRYN_NODE_H
