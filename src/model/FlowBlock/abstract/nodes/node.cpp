//
// Created by tanawin on 8/1/2567.
//
#include"node.h"

namespace kathryn{


    std::string NT_to_string(NODE_TYPE nt){

        std::string mapper[NODE_TYPE_CNT] = {
        "ASM_NODE",
        "STATE_NODE",
        "SYN_NODE",
        "PSEUDO_NODE",
        "DUMMY_NODE",
        "START_NODE",
        "WAITCOND_NODE",
        "WAITCYCLE_NODE"
        };
        return mapper[nt];

    }

    bool thereAreStateLessConnection(std::vector<Node*> nds, Node* startNode) {

        assert(!startNode->isStateFullNode());
        std::queue<Node*> toCheckingNode;

        /**fill checking node from nds*/
        toCheckingNode.push(startNode);

        while (!toCheckingNode.empty()){
            auto frontNode = toCheckingNode.front();
            toCheckingNode.pop();
            /** check depend matched node*/
            for (auto checkNode: nds){
                if (checkNode == frontNode){
                    return true;
                }
            }
            /**add dependNode to next iteration*/
            for (auto depNode : frontNode->getDependNodes()){
                assert(depNode != nullptr);
                if (!depNode->isStateFullNode()){
                    toCheckingNode.push(depNode);
                }
            }

        }

        return false;
    }






}