//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_ASMNODE_H
#define KATHRYN_ASMNODE_H

#include <utility>

#include "node.h"

namespace kathryn {

    struct AsmNode : Node {
        std::vector<AssignMeta*> _assignMetas; //// AssignMeta is must not use the same assign metas

        explicit AsmNode(AssignMeta *assignMeta) :
                Node(ASM_NODE),
                _assignMetas({assignMeta}){
            assert(assignMeta != nullptr);
        }

        explicit AsmNode(std::vector<AssignMeta*> assignMetas):
                Node(ASM_NODE),
                _assignMetas(std::move(assignMetas)){

            for (auto* asmMeta: _assignMetas){
                assert(asmMeta != nullptr);
            }

        }

        ~AsmNode() override = default;

        void assign() override{
            assert(false);
        }

        void assignFromStateNode(){
            assert(nodeSrcs.size() == 1);
            assert(nodeSrcs[0].condition == nullptr);
            assert(nodeSrcs[0].dependNode != nullptr);
            assert(!_assignMetas.empty());

            for (auto* assignMeta: _assignMetas) {

                /*** for reg <<= operator*/
                if (assignMeta->asmType == ASM_DIRECT){
                    ///////////// assign from current dependency
                    auto resultUpEvent = new UpdateEvent({
                        nodeSrcs[0].condition,
                        nodeSrcs[0].dependNode->getExitOpr(),
                        &assignMeta->valueToAssign,
                        assignMeta->desSlice,
                        DEFAULT_UE_PRI_USER
                    });
                    assignMeta->updateEventsPool.push_back(resultUpEvent);

                    ////////////////////////////////////////////////////////////////////////////////////////
                /** for reg = operator*/
                }else if (assignMeta->asmType == ASM_EQ_DEPNODE){

                    //////////////// assign as same as node that have been assign
                    for (auto nodeSrc: nodeSrcs[0].dependNode->nodeSrcs) {
                        auto resultUpEvent = new UpdateEvent({
                                                                     nodeSrc.condition,
                                                                     nodeSrc.dependNode->getExitOpr(),
                                                                     &assignMeta->valueToAssign,
                                                                     assignMeta->desSlice,
                                                                     DEFAULT_UE_PRI_USER
                                                             });
                        assignMeta->updateEventsPool.push_back(resultUpEvent);
                    }
                }
            }

            /*** no need to deal with rst event due to data self invoked*/
        }
        /** assign with no flow block related*/
        void dryAssign() override{
            assert(!_assignMetas.empty());
            for (auto* assignMeta: _assignMetas) {
                auto resultUpEvent = new UpdateEvent({
                                                             nullptr,
                                                             nullptr,
                                                             &assignMeta->valueToAssign,
                                                             assignMeta->desSlice,
                                                             DEFAULT_UE_PRI_USER
                                                     });

                assignMeta->updateEventsPool.push_back(resultUpEvent);
            }
        }

        int getCycleUsed() override { return 1; }

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            /** for basic assignment log engine is irrelevant*/
            /////incEngine(false) do not increase;
        }



    };

}
#endif //KATHRYN_ASMNODE_H
