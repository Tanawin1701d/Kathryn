//
// Created by tanawin on 2/12/2566.
//

#ifndef KATHRYN_FLOWBLOCK_BASE_H
#define KATHRYN_FLOWBLOCK_BASE_H

#include<memory>
#include<vector>

#include "model/hwComponent/abstract/assignable.h"
#include "model/hwComponent/register/register.h"
#include "model/hwComponent/expression/expression.h"
#include "model/hwComponent/abstract/operation.h"

namespace kathryn {



    /** atomic node for assigning */
    struct Node{
    ///// f
    bool psudoNode = false;
    Operable* psudoAssignMeta;
    AssignMeta* assignMeta;
    Operable* condition = nullptr;
    Operable* dependState = nullptr;

    Node(): psudoNode(true), assignMeta(nullptr){}

    explicit Node(AssignMeta* asmMeta): assignMeta(asmMeta){}

//    Node(const Node& rhs){
//        assignMeta = rhs.assignMeta;
//        condition  = rhs.condition;
//        dependState = rhs.dependState;
//    }

    static void addLogic(Operable* desLogic,Operable* opr, LOGIC_OP op){
        assert(op == BITWISE_AND || op == BITWISE_OR);
        assert(opr != nullptr);
        if(desLogic == nullptr){
            desLogic = opr;
            return;
        }

        if(op == BITWISE_AND){
            desLogic = &((*desLogic) & (*opr));
        }else if (op == BITWISE_OR){
            desLogic = &((*desLogic) | (*opr));
        }
    }

    void addCondtion(Operable* opr, LOGIC_OP op){
        addLogic(condition, opr, op);
    }

    void addDependState(Operable* opr, LOGIC_OP op){
        addLogic(dependState, opr, op);
    }

    void assign(){
        if (psudoNode){
            *psudoAssignMeta = *dependState & *condition;
        }else {
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

    };

    /* use for inner block return to outter block**/
    struct NodeWrap {
    public:
        /** entrance represent UpdateEvent which refer to node wire that be head of the subblock*/
        /** note that expression must not be here due to the abstract of the system*/
        std::vector<Node*> entranceNodes;
        /** the exit condition that allow next building block run*/
        Operable* exitOpr = nullptr;

        void addEntraceNode(Node* nd){
            assert(nd != nullptr);
            entranceNodes.push_back(nd);
        }
        void addExitOpr(Operable* opr){
            assert(opr != nullptr);
            exitOpr = opr;
        }

        NodeWrap& operator = (const NodeWrap& rhs){
            ///// change node location
            for (auto nd: rhs.entranceNodes){
                Node* preInsert = new Node(*nd);
                entranceNodes.push_back(preInsert);
            }
            exitOpr = rhs.exitOpr;
            return *this;
        }

        NodeWrap(const NodeWrap& rhs){
            exitOpr = nullptr;
            *this = rhs;
        }

        NodeWrap(){};

    };

    /** it is basic node that only have one event at a node */
    class Controller;
    typedef std::shared_ptr<Controller> ControllerPtr;


    enum FLOW_BLOCK_TYPE{
        SEQUENTIAL,
        PARALLEL,
        IF,
        ELIF,
        ELSE,
        WHILE,
        DO_WHILE,
        DUMMY_BLOCK
    };

    class FlowBlockBase {
    protected:
        std::vector<FlowBlockBase*> subBlocks;
        std::vector<Node*> basicNodes;
        FLOW_BLOCK_TYPE _type;
        ControllerPtr ctrl;
        /** generate implicit subblock typically used with if and while block*/
        FlowBlockBase* genImplicitSubBlk(FLOW_BLOCK_TYPE defaultType);
    public:
        explicit FlowBlockBase(FLOW_BLOCK_TYPE type);
        virtual ~FlowBlockBase();

        /**
         * entrance to make controller interact with
         * */
        /** when basic behavior describe in flow block*/
        virtual void addElementInFlowBlock(Node* node){
            assert(node != nullptr);
            basicNodes.push_back(node);
        };
        /** when inside complex element such as sub flow block is finish, user must add here*/
        virtual void addSubFlowBlock(FlowBlockBase* subBlock){
            assert(subBlock != nullptr);
            subBlocks.push_back(subBlock);
        };
        /** when every thing is finish call this to get sumarisation*/
        virtual NodeWrap* sumarizeBlock() = 0;

        /**
         * communicator to controller
         * */
         /** todo make it communicate with controller */
         virtual void onAttachBlock() = 0; //// it is supposed to acknowledge controller whether this block is declared
         virtual void onDetachBlock() = 0;

         FLOW_BLOCK_TYPE getFlowType(){return _type;}

         /**
          * for module communicate with
          * */
          virtual void buildHwComponent() = 0;



    };




}

#endif //KATHRYN_FLOWBLOCK_BASE_H
