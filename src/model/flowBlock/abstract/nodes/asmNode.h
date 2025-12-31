//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_ASMNODE_H
#define KATHRYN_ASMNODE_H

#include <utility>

#include "node.h"
#include "model/controller/asmMode.h"

namespace kathryn {


    struct AsmNode : Node {
        std::vector<AssignMeta*> _assignMetas; //// AssignMeta is must not use the same assign metas

        ////// it will used as order of the assignment node
        ////// when there are multiple assigns in the system
        //// TODO add per element metadata

        explicit AsmNode(AssignMeta *assignMeta) :
                Node(ASM_NODE),
                _assignMetas({assignMeta})
        {
            assert(assignMeta != nullptr);
            setClockMode(CM_CLK_UNUSED);

        }

        explicit AsmNode(std::vector<AssignMeta*> assignMetas):
                Node(ASM_NODE),
                _assignMetas(std::move(assignMetas)){

            for (auto* asmMeta: _assignMetas){
                assert(asmMeta != nullptr);

            }
            setClockMode(CM_CLK_UNUSED);

        }

        ~AsmNode()  = default;

        std::vector<AssignMeta*>& getAssignMetas(){return _assignMetas;}

        void assign() override{
            assert(false);
        }

        void transferOutAssignMetaOwnership(){_assignMetas.clear();}

        void addSpecificPreCondition(Operable* cond, int desIdx){

            assert(cond != nullptr);
            assert(desIdx >= 0 && desIdx < _assignMetas.size());
            _assignMetas[desIdx]->addSpecificPreCondition(cond);

        }

        void overrideClockMode(CLOCK_MODE mode){
            /** for asmNode it is first assign with it was built however; the flowblock should override the system
             * before the assignment is building*/
            for (auto* assignMeta: _assignMetas){
                //assignMeta->clockMode = mode;
            }
        }

        void assignFromStateNode(Operable* holdSignal, Operable* resetSignal){
            assert(nodeSrcs.size() == 1);
            //assert(nodeSrcs[0].condition == nullptr);
            assert(nodeSrcs[0].dependNode != nullptr);
            assert(!_assignMetas.empty());

            for (int assignIdx = 0; assignIdx < _assignMetas.size(); assignIdx++) {
                AssignMeta* assignMeta     = _assignMetas[assignIdx];
                /*** for reg <<= operator*/
                if (assignMeta->asmType == ASM_DIRECT){

                    ////// bind node condition with pre_condition first\
                    ////////// handle condition and reset signal
                    Operable* condEvent = addLogicWithOutput(nodeSrcs[0].condition, nullptr, BITWISE_AND);
                    if (holdSignal != nullptr){
                        condEvent = addLogicWithOutput(condEvent, &(~(*holdSignal)), BITWISE_AND);
                    }
                    if (resetSignal != nullptr){
                        condEvent = addLogicWithOutput(condEvent, &(~(*resetSignal)), BITWISE_AND);
                    }
                    ////////// handle condition with state
                    condEvent = addLogicWithOutput(condEvent, nodeSrcs[0].dependNode->getStateOperating(), BITWISE_AND);
                    ///////////// assign from current dependency
                    assert(assignMeta->preUpdateElement != nullptr);

                    /////// update new Condition
                    UpdateEventCond* updateWithState = new UpdateEventCond();
                    updateWithState->addSubStmt(condEvent, assignMeta->getCurrentEvent());
                    assignMeta->setNewEditingEvent(updateWithState);
                    assignMeta->finalUpdate();
                    // auto resultUpEvent = new UpdateEvent({
                    //     condEvent,
                    //     nodeSrcs[0].dependNode->getStateOperating(), /////// you can not simply use genExitOpr because it is used to colab with reset
                    //     &assignMeta->valueToAssign,
                    //     assignMeta->desSlice,
                    //     _asmPriority,
                    //     _asmId,
                    //     assignMeta->clockMode
                    // });
                    ////////////////////////////////////////////////////////////////////////////////////////
                /** for reg = operator*/
                }else if (assignMeta->asmType == ASM_EQ_DEPNODE){
                    //////////////// assign as same as node that have been assign
                    for (auto nodeSrc: nodeSrcs[0].dependNode->nodeSrcs) {

                        Operable* condEvent = addLogicWithOutput(nodeSrc.condition, nullptr, BITWISE_AND);
                        if (holdSignal != nullptr){
                            condEvent = addLogicWithOutput(condEvent, &(~(*holdSignal)), BITWISE_AND);
                        }
                        condEvent = addLogicWithOutput(condEvent, nodeSrc.dependNode->getExitOpr(), BITWISE_AND);

                        UpdateEventCond* updateWithState = new UpdateEventCond();
                        updateWithState->addSubStmt(condEvent, assignMeta->getCurrentEvent());
                        assignMeta->setNewEditingEvent(updateWithState);
                        assignMeta->finalUpdate();
                        // auto resultUpEvent = new UpdateEvent({
                        //      condEvent,
                        //      nodeSrc.dependNode->getExitOpr(), ///// it is supposed to sensitive to reset already?
                        //      &assignMeta->valueToAssign,
                        //      assignMeta->desSlice,
                        //      _asmPriority,
                        //      _asmId,
                        //      assignMeta->clockMode
                        // });
                        // assignMeta->updateEventsPool.push_back(resultUpEvent);
                    }
                }else{
                    assert(false);
                }
            }

            /*** no need to deal with rst event due to data self invoked*/
        }
        /** assign with no flow block related*/
        void dryAssign() override{
            assert(!_assignMetas.empty());
            assert(nodeSrcs.empty());


            for (int assignIdx = 0; assignIdx < _assignMetas.size(); assignIdx++) {
                AssignMeta* assignMeta = _assignMetas[assignIdx];
                UpdateEventCond* updateWithPureCondState = new UpdateEventCond();
                updateWithPureCondState->addSubStmt(nullptr, assignMeta->getCurrentEvent());
                assignMeta->setNewEditingEvent(updateWithPureCondState);
                assignMeta->finalUpdate();
                //
                // auto resultUpEvent = new UpdateEvent({
                //                                              condEvent,
                //                                              nullptr,
                //                                              &assignMeta->valueToAssign,
                //                                              assignMeta->desSlice,
                //                                              _asmPriority,
                //     _asmId,
                // assignMeta->clockMode});
                //
                // assignMeta->updateEventsPool.push_back(resultUpEvent);
            }
        }

        int getCycleUsed() override { return 1; }

    };

}
#endif //KATHRYN_ASMNODE_H
