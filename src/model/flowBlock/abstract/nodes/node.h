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
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/spReg/syncReg.h"
#include "model/debugger/modelDebugger.h"
#include "model/simIntf/flowBlock/flowBlockSimEngine.h"
#include "nodeCon.h"

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
        INTERRUPT_NODE,
        NODE_TYPE_CNT
    };




    /***
     * NODE operation requirement
     * 1. add dep
     * 2.
     * **/

    std::string NT_to_string(NODE_TYPE nt);


    struct SrcNodeAgent;

    struct Node : public ModelDebuggable,
                  public FlowSimEngine{
        NODE_TYPE          nodeType      = NODE_TYPE_CNT;
        std::string        identName     = "NODE_UNNAME";
        std::vector<NODE_META> dep[CON_NODE_CNT];

        /** constructor deconstructor*/
        explicit Node(NODE_TYPE nt):FlowSimEngine(),nodeType(nt){};
                 Node(Node& rhs) = delete;
                ~Node() override = default;
        /*** node type*/
        NODE_TYPE getNodeType() const{return nodeType;}
        /** gen src node agent*/
        SrcNodeAgent genSrcAgent();
        /** this is master function to allocate dependency to slave***/
        virtual void allocDep(CONNECT_NODE_PURPOSE pp,
                              Operable* condition,
                              Node* state){
            dep[pp].emplace_back(condition, state);
        }

        bool checkAllDepEmpty(){
            for (int depType = 0; depType < CON_NODE_CNT; depType++){
                if (!dep[depType].empty()){
                    return false;
                }
            }
            return true;
        }

        virtual void      finalize() = 0;
        /** provided src state data*/
        virtual Operable* getExitOpr()   = 0;
        /** cycle that is use in this node*/
        virtual int       getCycleUsed() = 0;
        /** is Stateful node (reffer to node that consume at least 1 cycle from machine)*/
        virtual bool isStateFullNode()   = 0;



        /** get debugger value*/

        std::string getMdIdentVal() override{
            std::string ret = NT_to_string(nodeType) + " @ " + std::to_string((ull)this);
            return ret;
        }
        void addMdLog(MdLogVal* mdLogVal) override{
            ////// TODO make mdlog
        }
        /*** simulation override*/
        void simExitCurCycle() override{
            unsetBlockOrNodeRunning();
            unSetSimStatus();
        }
        /** internal value identifier for debugging purpose*/
        void setInternalIdent(std::string identVal){
            identName = identVal;
        }

    };


    /***
     *
     * this is agent that is used to interact with node (src side)
     *
     * */
    struct SrcNodeAgent{

        /** condition that trigger des Node*/
        Operable* _condition = nullptr;
        Node*     _desNode   = nullptr;

        SrcNodeAgent(Operable* cond, Node* desNode);

        void addDep(Node* srcNode, CONNECT_NODE_PURPOSE cnp) const;

        void addCond(Operable* cond, LOGIC_OP op);

        void finalize() const;


    };

    void addLogic(Operable* &desLogic, Operable *opr, LOGIC_OP op);




}

#endif //KATHRYN_NODE_H
