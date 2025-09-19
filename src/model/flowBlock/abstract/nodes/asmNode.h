//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_ASMNODE_H
#define KATHRYN_ASMNODE_H

#include <utility>

#include "node.h"

namespace kathryn {


    struct AsmNode : Node {
        static inline ull ASM_NODE_CNT = 0;
        std::vector<AssignMeta*> _assignMetas; //// AssignMeta is must not use the same assign metas
        Operable*                _preCondition = nullptr;
        ull                      _asmId;
        ////// it will used as order of the assignment node
        ////// when there are multiple assigns in the system
        //// TODO add per element metadata
        std::vector<Operable*>   _preCondPerMetas;

        explicit AsmNode(AssignMeta *assignMeta) :
                Node(ASM_NODE),
                _assignMetas({assignMeta}),
                _asmId(ASM_NODE_CNT++),
                _preCondPerMetas(_assignMetas.size(), nullptr)

        {
            assert(assignMeta != nullptr);

        }

        explicit AsmNode(std::vector<AssignMeta*> assignMetas):
                Node(ASM_NODE),
                _assignMetas(std::move(assignMetas)),
                _asmId(ASM_NODE_CNT++),
                _preCondPerMetas(_assignMetas.size(), nullptr){

            for (auto* asmMeta: _assignMetas){
                assert(asmMeta != nullptr);
            }

        }

        ~AsmNode()  = default;

        void assign() override{
            assert(false);
        }

        void assignFromStateNode(Operable* holdSignal){
            assert(nodeSrcs.size() == 1);
            assert(nodeSrcs[0].condition == nullptr);
            assert(nodeSrcs[0].dependNode != nullptr);
            assert(!_assignMetas.empty());

            for (int assignIdx = 0; assignIdx < _assignMetas.size(); assignIdx++) {
                AssignMeta* assignMeta     = _assignMetas[assignIdx];
                Operable*   preCondPerMeta = _preCondPerMetas[assignIdx];
                /*** for reg <<= operator*/
                if (assignMeta->asmType == ASM_DIRECT){

                    ////// bind node condition with pre_condition first\

                    Operable* condEvent = addLogicWithOutput(nodeSrcs[0].condition, _preCondition, BITWISE_AND);
                    if (holdSignal != nullptr){
                        condEvent = addLogicWithOutput(condEvent, &(~(*holdSignal)), BITWISE_AND);
                    }
                    condEvent = addLogicWithOutput(condEvent, preCondPerMeta, BITWISE_AND);

                    ///////////// assign from current dependency
                    auto resultUpEvent = new UpdateEvent({
                        condEvent,
                        nodeSrcs[0].dependNode->getStateOperating(), /////// you can not simply use genExitOpr because it is used to colab with reset
                        &assignMeta->valueToAssign,
                        assignMeta->desSlice,
                        DEFAULT_UE_PRI_USER,
                        _asmId
                    });
                    assignMeta->updateEventsPool.push_back(resultUpEvent);

                    ////////////////////////////////////////////////////////////////////////////////////////
                /** for reg = operator*/
                }else if (assignMeta->asmType == ASM_EQ_DEPNODE){
                    //////////////// assign as same as node that have been assign
                    for (auto nodeSrc: nodeSrcs[0].dependNode->nodeSrcs) {

                        Operable* condEvent = addLogicWithOutput(nodeSrc.condition, _preCondition, BITWISE_AND);
                        if (holdSignal != nullptr){
                            condEvent = addLogicWithOutput(condEvent, &(~(*holdSignal)), BITWISE_AND);
                        }
                        condEvent = addLogicWithOutput(condEvent, preCondPerMeta, BITWISE_AND);


                        auto resultUpEvent = new UpdateEvent({
                             condEvent,
                             nodeSrc.dependNode->getExitOpr(),
                             &assignMeta->valueToAssign,
                             assignMeta->desSlice,
                             DEFAULT_UE_PRI_USER,
                             _asmId});
                        assignMeta->updateEventsPool.push_back(resultUpEvent);
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
                Operable* preCondPerMeta = _preCondPerMetas[assignIdx];
                Operable* condEvent = addLogicWithOutput(_preCondition, preCondPerMeta, BITWISE_AND);
                auto resultUpEvent = new UpdateEvent({
                                                             condEvent,
                                                             nullptr,
                                                             &assignMeta->valueToAssign,
                                                             assignMeta->desSlice,
                                                             DEFAULT_UE_PRI_USER});

                assignMeta->updateEventsPool.push_back(resultUpEvent);
            }
        }

        int getCycleUsed() override { return 1; }

        void addPreCondition(Operable* cond, LOGIC_OP op){
            assert(cond != nullptr);
            addLogic(_preCondition, cond, op);
        }

        void addSpecificPreCondition(Operable* cond, LOGIC_OP op, int idx){
            assert(cond != nullptr);
            assert(idx >= 0 && idx < _preCondPerMetas.size());
            addLogic(_preCondPerMetas[idx], cond, op);
        }



    };

}
#endif //KATHRYN_ASMNODE_H
