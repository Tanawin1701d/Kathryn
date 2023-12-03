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

namespace kathryn {

    struct NodeWrapper {
    private:
        /** entrance represent UpdateEvent which refer to node wire that be head of the subblock*/
        /** note that expression must not be here due to the abstract of the system*/
        std::vector<std::shared_ptr<UpdateEvent>> entranceElements;
        /** the exit condition that allow next building block run*/
        expressionPtr exitExpr;

    };

    /** it is basic node that only have one event at a node */
    struct Node{
    private:
        std::shared_ptr<UpdateEvent> updateElement; /** the register or wire that update in this cycle*/
    public:
        explicit Node(): updateElement(std::make_shared<UpdateEvent>()){}
        explicit Node(const std::shared_ptr<UpdateEvent>& ue): updateElement(ue){}


    };

    class Controller;
    typedef std::shared_ptr<Controller> ControllerPtr;


    enum FLOW_BLOCK_TYPE{
        SEQUENTIAL,
        PARALLEL,
        STATE_IF,
        NO_STATE_IF,
        WHILE,
        DO_WHILE,

    };

    class FlowBlockBase;

    typedef std::shared_ptr<FlowBlockBase> FlowBlockBasePtr;

    class FlowBlockBase {
    protected:
        FLOW_BLOCK_TYPE _type;
        ControllerPtr ctrl;
    public:
        explicit FlowBlockBase(FLOW_BLOCK_TYPE type): _type(type){};
        /**
         * entrance to make controller interact with
         * */
        /** when basic behavior describe in flow block*/
        virtual void addElementInFlowBlock(std::shared_ptr<Node> node) = 0;
        /** when inside complex element such as sub flow block is finish, user must add here*/
        virtual void addElementInFlowBlock(FlowBlockBasePtr subBlock) = 0;
        /** when every thing is finish call this to get sumarisation*/
        virtual std::shared_ptr<NodeWrapper> sumarizeBlock() = 0;

        /**
         * communicator to controller
         * */
         /** todo make it communicate with controller */
         virtual void onAttachBlock() = 0; //// it is supposed to acknowledge controller whether this block is declared
         virtual void onDetachBlock() = 0;

         FLOW_BLOCK_TYPE getFlowType(){return _type;}



    };




}

#endif //KATHRYN_FLOWBLOCK_BASE_H
