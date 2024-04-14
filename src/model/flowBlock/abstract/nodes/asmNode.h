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

        /** constructor*/
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


        AsmNode(const AsmNode &other) = delete;

        ~AsmNode() = default;

        bool isThereIndirectAsmMeta() {

            for (auto* assignMeta: _assignMetas) {
                if (assignMeta->asmType == ASM_EQ_DEPNODE) {
                    return true;
                }
            }
            return false;


        }

        void finalize() override{ assert(false);}

        void finalizeFromStateNode(){
            /***
             *
             *
             *
             *
             * TODO make fire condition and state
             *
             *
             *
             *
             *
             *
             * */
        }
        /** assign with no flow block related*/
        void dryFinalize(){
            assert(!_assignMetas.empty());
            for (auto* assignMeta: _assignMetas) {
                auto resultUpEvent = new UpdateEvent(
                                                             nullptr,
                                                             nullptr,
                                                             &assignMeta->valueToAssign,
                                                             assignMeta->desSlice,
                                                             DEFAULT_UE_PRI_USER
                                                     );

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
