//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_NODE_H
#define KATHRYN_NODE_H

#include<vector>
#include<memory>
#include<queue>
#include<map>

#include "model/hwComponent/abstract/operation.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/FlowBlock/abstract/spReg/stateReg.h"
#include "model/FlowBlock/abstract/spReg/syncReg.h"
#include "model/debugger/modelDebugger.h"



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

    struct Node : public ModelDebuggable,
                  public FlowSimEngineMaster{
        Node*    srcCpyNode = nullptr;
        NODE_TYPE nodeType  = NODE_TYPE_CNT;
        Operable* condition = nullptr;
        std::vector<Node*> dependNodes;
        LOGIC_OP dependStateRaiseCond = OP_DUMMY;

        Node(Node& rhs):
                FlowSimEngineMaster(new FlowSimEngine())
        {
            nodeType             = rhs.nodeType;
            condition            = rhs.condition;
            dependNodes          = rhs.dependNodes;
            dependStateRaiseCond = rhs.dependStateRaiseCond;
        }

        explicit Node(NODE_TYPE nt):
                FlowSimEngineMaster(new FlowSimEngine()),
                nodeType(nt),
                condition(nullptr),
                dependStateRaiseCond(OP_DUMMY){};

        ~Node() override = default;

        virtual Node* clone() = 0;

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
        Operable* transformAllDepNodeToOpr(){
            Operable* resultOpr = nullptr;
            for (auto nd : dependNodes){
                addLogic(resultOpr, nd->getExitOpr(), dependStateRaiseCond);
            }
            assert(resultOpr != nullptr);
            return resultOpr;
        }

        /**
         * function that allow sp node custom their behavior
         * **/

        /** set that how to join node to make this node valid*/
        virtual void setDependStateJoinOp(LOGIC_OP op){
            dependStateRaiseCond = op;
        }
        /** unset event when state is raised there must be condition that bring this down*/
        virtual void makeUnsetStateEvent(){assert(false);}
        virtual void makeRstAndDefStateEvent(){assert(false);}
        /** provided src state data*/
        virtual Operable* getExitOpr(){ return nullptr; };
        /** assign value with proper condition*/
        virtual void assign() = 0; /** please make sure that makeunsetState is called*/
        /** cycle that is use in this node*/
        virtual int getCycleUsed() = 0;
        /** is Stateful node (reffer to node that consume at least 1 cycle from machine)*/
        virtual bool isStateFullNode(){ return true; }



        /** get debugger value*/
        void setCpyPtr(Node* srcPtr){
            srcCpyNode = srcPtr;
        }

        std::string getMdDescribe() override{
            std::string ret = "  have node dep [ ";
            for (auto depNode : dependNodes){
                ret += depNode->getMdIdentVal();
                ret += ", ";
            }
            /** condition*/
            ret += "] with condition [ ";
            if (condition != nullptr) {
                ret += condition->castToIdent()->getIdentDebugValue();
            }
            ret += " ] with dep join condition [ ";
            ret += lop_to_string(dependStateRaiseCond);
            ret += " ] ";

            if (srcCpyNode != nullptr){
                ret += "cpyFrom [";
                ret += srcCpyNode->getMdIdentVal();
                ret += " ]";

            }
            return ret;
        }

        std::string getMdIdentVal() override{
            std::string ret = NT_to_string(nodeType) + " @ " + std::to_string((ull)this);
            return ret;
        }

        void addMdLog(MdLogVal* mdLogVal) override{
            mdLogVal->addVal("[Node] " + getMdIdentVal() +  "have node dep");
            for (auto depNode : dependNodes){
                mdLogVal->addVal(depNode->getMdIdentVal());
            }
            mdLogVal->addVal("----> condition");
            if (condition != nullptr) {
                mdLogVal->addVal(condition->castToIdent()->getIdentDebugValue());
            }
            mdLogVal->addVal("----> dep join condition");
            mdLogVal->addVal(lop_to_string(dependStateRaiseCond));
        }

        /*** simulation override*/

        //////void prepareSim() override{};

        void simExitCurCycle() override{
            unsetBlockOrNodeRunning();
            unSetSimStatus();
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
