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

#include "model/controller/clockMode.h"
#include "model/hwComponent/abstract/operation.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/abstract/assignable.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/spReg/syncReg.h"
#include "model/debugger/modelDebugger.h"
#include "sim/modelSimEngine/flowBlock/flowBaseSim.h"


namespace kathryn {

    struct NodeWrap;

    enum NODE_TYPE{
        ASM_NODE,
        STATE_NODE,
        SYN_NODE,
        PSEUDO_NODE,
        DUMMY_NODE,
        OPR_NODE,
        START_NODE,
        WAITCOND_NODE,
        WAITCYCLE_NODE,
        NODE_TYPE_CNT
    };

    constexpr int NODE_CYCLE_USED_UNKNOWN = -1;


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

    struct Node : public ModelDebuggable
                {
        NODE_TYPE                nodeType  = NODE_TYPE_CNT;
        std::vector<NodeSrcEdge> nodeSrcs;
        std::string              identName = "NODE_UNNAME";

        Node*                    intReset  = nullptr;
        Node*                    holdNode  = nullptr;

        CLOCK_MODE               clkMode = CM_CLK_FREE; //// indicate the clk sensitivity type
        /// for asm node it should be assigned since the was built
        /// for other flowblock should assign it

        /** simulate support register*/
        std::vector<CtrlFlowRegBase*> relatedCycleConsumeReg;

        Node(Node& rhs) = delete;

        explicit Node(NODE_TYPE nt):
                nodeType(nt){};

        ~Node(){};
        NODE_TYPE getNodeType() const{
            return nodeType;
        }

        static void addLogic(Operable* &desLogic, Operable *opr1, LOGIC_OP op) {
            assert(op == BITWISE_AND || op == BITWISE_OR);
            assert(opr1 != nullptr);
            if (desLogic == nullptr) {
                desLogic = opr1;
                return;
            }

            if (op == BITWISE_AND) {
                desLogic = &((*desLogic) & (*opr1));
            } else if (op == BITWISE_OR) {
                desLogic = &((*desLogic) | (*opr1));
            }
        }

        static Operable* addLogicWithOutput(Operable* opr1, Operable* opr2, LOGIC_OP op){
            assert(op == BITWISE_AND || op == BITWISE_OR);

            if ( (opr1 == nullptr) && (opr2 == nullptr)){
                return nullptr;
            }

            if(opr1 == nullptr){
                return opr2;
            }else if(opr2 == nullptr){
                return opr1;
            }else if(op == BITWISE_AND) {
                return &((*opr1) & (*opr2));
            }else if(op == BITWISE_OR) {
                return &((*opr1) | (*opr2));
            }else{
                assert(false);
            }
        }

        /** add dependNode for assignment*/
        void addDependNode(Node* srcNode, Operable* cond) {
            assert(srcNode != nullptr);
            nodeSrcs.push_back({srcNode, cond});
        }
        /** add to relatedCycleConsumeReg which is used to identify register that related to cycle usage*/
        void addCycleRelatedReg(CtrlFlowRegBase* ctrlReg){
            assert(ctrlReg != nullptr);
            relatedCycleConsumeReg.push_back(ctrlReg);
        }

        std::vector<CtrlFlowRegBase*>& getCycleRelatedReg(){
            return relatedCycleConsumeReg;
        }

        std::vector<NodeSrcEdge>& getDependNodes() {return nodeSrcs;}

        /**
         * clock
         *
         */
        void       setClockMode(CLOCK_MODE cm){ clkMode = cm;}
        CLOCK_MODE getClockMode() const{ return clkMode;}

        /**
         * interrupt handler
         */
        ///// interrupt reset
        void setInterruptReset(Node* rst){
            assert(rst != nullptr);
            intReset = rst;
        }

        bool isThrereIntReset  () const{ return intReset != nullptr; }

        Node* getInterruptReset() const{ return intReset; }

        Operable* bindWithRstOutPutIfReset(Operable* rawExit){
            assert(rawExit != nullptr);
            assert(rawExit->getOperableSlice().getSize() == 1);
            if (isThrereIntReset()){
                return &( (*rawExit) & (~(*getInterruptReset()->getExitOpr())) );
            }
            return rawExit;
        }
        ///// hold signal
        void setHold(Node* hn){ /// hold node
            assert(hn != nullptr);
            assert(holdNode == nullptr); /// we do not allow override the dsame node
            holdNode = hn;
        }

        bool  isThereHold()  const { return holdNode != nullptr; }

        Node* getHoldNode() const { return holdNode; }

        Operable* bindWithHoldIfHold(Operable* rawExit){
            assert(rawExit != nullptr);
            assert(rawExit->getOperableSlice().getSize() == 1);

            if (isThereHold()){
                return &((*rawExit) & (~(*getHoldNode()->getExitOpr())));
            }
            return rawExit;
        }



        /**
         * function that allow sp node custom their behavior
         * **/
        /** unset event when state is raised there must be condition that bring this down*/
        virtual void      makeUnsetStateEvent(){assert(false);}
        virtual void      makeUserResetEvent(){assert(false);}
        /** provided src state data*/
        virtual Operable* getExitOpr(){ return nullptr; };
        virtual Operable* getStateOperating(){return nullptr;}
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
        /** internal value identifier for debugging purpose*/
        void setInternalIdent(std::string identVal){
            identName = std::move(identVal);
        }



    };

}

#endif //KATHRYN_NODE_H
