//
// Created by tanawin on 12/1/2567.
//

#ifndef KATHRYN_ASMNODE_H
#define KATHRYN_ASMNODE_H

#include "node.h"

namespace kathryn {

    struct AsmNode : Node {
        AssignMeta *_assignMeta = nullptr; //// AssignMeta is must not use the same assign metas

        explicit AsmNode(AssignMeta *assignMeta) :
                Node(ASM_NODE),
                _assignMeta(assignMeta) {
            assert(_assignMeta != nullptr);
        }

        AsmNode(const AsmNode &other) : Node((Node &) other) {
            _assignMeta = other._assignMeta;
        }

        Node *clone() override {
            auto clNode = new AsmNode(*this);
            return clNode;
        }

        void assign() override {
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

}
#endif //KATHRYN_ASMNODE_H
