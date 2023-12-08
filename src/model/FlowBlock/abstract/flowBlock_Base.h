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
    /** it is basic node that only have one event at a node */
    class Controller;
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

    /** atomic node for assigning */
    struct Node{

    bool        pseudoNode      = false;
    Operable*   psudoAssignMeta = nullptr;
    AssignMeta* assignMeta      = nullptr; //// AssignMeta is must not use the same assign meta
    Operable*   condition       = nullptr;
    Operable*   dependState     = nullptr;

    /** This constructor is use for psuedo Node */
    Node(): pseudoNode(true), assignMeta(nullptr),
            psudoAssignMeta(new expression()){}
    Node(Node& rhs){ operator=(rhs); }
    explicit Node(AssignMeta* asmMeta): assignMeta(asmMeta){}
    ~Node(){ delete assignMeta; }

    Node& operator = (const Node& rhs){
        if (&rhs == this)
            return *this;
        pseudoNode       = rhs.pseudoNode;
        psudoAssignMeta  = rhs.psudoAssignMeta;
        assignMeta       = new AssignMeta(*rhs.assignMeta);
        condition        = rhs.condition;
        dependState      = rhs.dependState;
    }

    static void addLogic(Operable*& desLogic,Operable* opr, LOGIC_OP op){
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
    /** add condition for assignment*/
    void addCondtion(Operable* opr, LOGIC_OP op){
        addLogic(condition, opr, op);
    }
    /** add dependState for assignment*/
    void addDependState(Operable* opr, LOGIC_OP op){
        addLogic(dependState, opr, op);
    }
    /** assign data to register*/
    void assign(){
        if (pseudoNode){
            (*psudoAssignMeta) = *(Operable*)(&(*dependState & *condition));
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

        void addConditionToAllNode(Operable* cond, LOGIC_OP op){
            assert(cond != nullptr);
            for (auto node: entranceNodes){
                node->addCondtion(cond, op);
            }
        }

        /** copy node pointer to this wrap*/
        /// todo we will make it copy node if need but for now we don't
        void transferNodeFrom(NodeWrap* nw){
            assert(nw != nullptr);
            for (auto node: nw->entranceNodes){
                entranceNodes.push_back(node);
            }
        }

        NodeWrap& operator = (const NodeWrap& rhs){
            if (&rhs == this){
                return *this;
            }
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

        NodeWrap() = default;

    };



    class FlowBlockBase {
    protected:
        std::vector<FlowBlockBase*> subBlocks;
        std::vector<Node*>          basicNodes;
        FLOW_BLOCK_TYPE             _type;
        ControllerPtr               ctrl;
        bool                        lazyDeletedRequired = false;
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
        /**
         * For custome block
         * */
        /** when every thing is finish call this to get sumarisation*/
        virtual NodeWrap* sumarizeBlock() = 0;
        /*** communicator to controller*/
         virtual void onAttachBlock() = 0; //// it is supposed to acknowledge controller whether this block is declared
         virtual void onDetachBlock() = 0;
         /*** for module communicate with* */
          virtual void buildHwComponent() = 0;

        ////// getter/setter
        FLOW_BLOCK_TYPE getFlowType(){return _type;}
        std::vector<FlowBlockBase*>& getSubBlocks(){
            return subBlocks;
        }
        bool isLazyDelete() const{
            return lazyDeletedRequired;
        }
        void setLazyDelete() {
            lazyDeletedRequired = true;
        }

    };




}

#endif //KATHRYN_FLOWBLOCK_BASE_H
