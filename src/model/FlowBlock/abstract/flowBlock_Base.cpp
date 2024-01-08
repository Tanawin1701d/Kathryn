//
// Created by tanawin on 2/12/2566.
//

#include "flowBlock_Base.h"
#include "model/controller/controller.h"
#include "model/FlowBlock/seq/seq.h"

namespace kathryn{

    int nextFbIdx = 0;


    FlowBlockBase::FlowBlockBase(FLOW_BLOCK_TYPE type):
    _type(type),
    ctrl(getControllerPtr()),
    lazyDeletedRequired(false),
    _fbId(nextFbIdx++),
    areThereForceExit(false),
    forceExitNode(nullptr)
    {}

    FlowBlockBase::~FlowBlockBase(){
        for (auto basicNode : basicNodes){
            delete basicNode;
        }
        for (auto sub_fb: subBlocks){
            delete sub_fb;
        }
        delete forceExitNode;
        /////// it is safe to delete nullptr
    }

    FlowBlockBase* FlowBlockBase::genImplicitSubBlk(FLOW_BLOCK_TYPE defaultType) {
        /** determine next flow block*/
        FLOW_BLOCK_TYPE nextFbType = ctrl->get_top_pattern_flow_block_type();
        if (nextFbType == DUMMY_BLOCK){
            nextFbType = defaultType;
        }
        /** create subblock*/
        if ( (nextFbType == PARALLEL_NO_SYN) || (nextFbType == PARALLEL_AUTO_SYNC)){
            return new FlowBlockParNoSync();
        }else if (nextFbType == SEQUENTIAL){
            return new FlowBlockSeq();
        }else{
            assert(false); /** can't determine flow type*/
        }
    }

    void FlowBlockBase::genSumForceExitNode(std::vector<NodeWrap *> &nws) {
        for (auto nw : nws){
            areThereForceExit |= (nw->getForceExitNode() != nullptr);
        }
        if (areThereForceExit){
            forceExitNode = new PseudoNode;
            for (auto nw : nws){
                if (nw->getForceExitNode() != nullptr){
                    forceExitNode->addDependNode(nw->getForceExitNode());
                }
            }
            forceExitNode->setDependStateJoinOp(BITWISE_OR);
            forceExitNode->assign();

        }
    }

    std::string getDescribe(){
        assert(false);
    }


    std::string FBT_to_string(FLOW_BLOCK_TYPE fbt){
        std::string mapper[FLOW_BLOCK_COUNT] = {
                "SEQUENTIAL",
                "PARALLEL_AUTO_SYNC",
                "PARALLEL_NO_SYN",
                "IF",
                "ELIF",
                "ELSE",
                "WHILE",
                "EXITWHILE",
                "CONDWAIT",
                "CLKWAIT",
                "DO_WHILE",
                "DUMMY_BLOCK",
        };
        assert(fbt < FLOW_BLOCK_COUNT);
        return mapper[fbt];
    }


}