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

    std::string NT_to_string(NODE_TYPE nt);

    struct Node {
        NODE_TYPE nodeType = NODE_TYPE_CNT;
        Operable* condition = nullptr;
        std::vector<Node*> dependNodes;
        LOGIC_OP dependStateRaiseCond = OP_DUMMY;

        Node(Node& rhs) {
            nodeType             = rhs.nodeType;
            condition            = rhs.condition;
            dependNodes          = rhs.dependNodes;
            dependStateRaiseCond = rhs.dependStateRaiseCond;
        }

        explicit Node(NODE_TYPE nt):
            nodeType(nt),
            condition(nullptr),
            dependStateRaiseCond(OP_DUMMY){};

        virtual ~Node() = default;

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
        virtual void makeUnsetStateEvent(){ assert(false); };
        /** provided src state data*/
        virtual Operable* getExitOpr(){ return nullptr; };
        /** assign value with proper condition*/
        virtual void assign() = 0; /** please make sure that makeunsetState is called*/
        /** cycle that is use in this node*/
        virtual int getCycleUsed() = 0;
        /** is Stateful node (reffer to node that consume at least 1 cycle from machine)*/
        virtual bool isStateFullNode(){ return true; }
        /** get describe value*/
        virtual std::string getDescribe(){
            std::string ret = "[node type "+ NT_to_string(nodeType) + " " +
                              std::to_string((ull)this) +" ] set depend to ";
            for (auto dependNode : dependNodes){
                ret += std::to_string((ull) dependNode) + " ";
            }
            return ret;
        }
    };

    struct AsmNode : Node{
        AssignMeta* _assignMeta = nullptr; //// AssignMeta is must not use the same assign metas

        explicit AsmNode(AssignMeta* assignMeta) :
            Node(ASM_NODE),
            _assignMeta(assignMeta){
            assert(_assignMeta != nullptr);
        }

        AsmNode(const AsmNode& other): Node((Node&) other){
            _assignMeta = other._assignMeta;
        }

        Node* clone() override{
            auto clNode = new AsmNode(*this);
            return clNode;
        }

        void assign() override{
            auto resultUpEvent = new UpdateEvent({
                                                         condition,
                                                         transformAllDepNodeToOpr(),
                                                         &_assignMeta->valueToAssign,
                                                         _assignMeta->desSlice,
                                                         9
                                                 });
            _assignMeta->updateEventsPool.push_back(resultUpEvent);
        }

        int getCycleUsed() override { return 1; }

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
