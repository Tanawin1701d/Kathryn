//
// Created by tanawin on 2/12/2566.
//

#include "flowBlock_Base.h"

#include <utility>
#include "model/controller/controller.h"
#include "model/FlowBlock/seq/seq.h"

namespace kathryn{

    int nextFbIdx = 0;


    FlowBlockBase::FlowBlockBase(FLOW_BLOCK_TYPE type, FB_CTRL_COM_META fbCtrlComMeta):
            FlowIdentifiable(FBT_to_string(type)),
            FlowSimulatable(new FlowSimEngine()),
            /** flow element*/
            _type(type),
            ctrl(getControllerPtr()),
            lazyDeletedRequired(false),
            _fbId(nextFbIdx++),
            /** controller communication policy*/
            _fbCtrlComMeta(std::move(fbCtrlComMeta)),
            /** exit management*/
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
        for (auto con_fb: conBlocks){
            delete con_fb;
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
            return new FlowBlockParAuto();
        }else if (nextFbType == SEQUENTIAL){
            return new FlowBlockSeq();
        }else{
            assert(false); /** can't determine flow type*/
        }
    }

    Operable* FlowBlockBase::purifyCondition(Operable* rawOpr){
            assert(rawOpr != nullptr);
            Slice rawSl = rawOpr->getOperableSlice();
            assert(rawSl.getSize() != 0);

            if (rawSl.getSize() == 1){
                return rawOpr;
            }
            makeVal(autoCmpCondVal, rawSl.getSize(), 0);
            return &((*rawOpr) > autoCmpCondVal);

    }

    void FlowBlockBase::genSumForceExitNode(std::vector<NodeWrap *> &nws) {
        for (auto nw : nws){
            areThereForceExit |= (nw->getForceExitNode() != nullptr);
        }
        if (areThereForceExit){
            forceExitNode = new PseudoNode(1);
            for (auto nw : nws){
                if (nw->getForceExitNode() != nullptr){
                    forceExitNode->addDependNode(nw->getForceExitNode());
                }
            }
            forceExitNode->setDependStateJoinOp(BITWISE_OR);
            forceExitNode->assign();

        }
    }

//    std::string FlowBlockBase::getDescribe(){
//        assert(false);
//    }

    std::vector<FlowBlockBase::sortEle> FlowBlockBase::sortSubAndConFbInOrder() {

        std::vector<sortEle> poolEle;
        /***pool sub block in to the array*/
        for(int i = 0; i < subBlocks.size(); i++){
            poolEle.push_back({subBlocks[i], subBlocksOrder[i]});
        }
        for(int i = 0; i < conBlocks.size(); i++){
            poolEle.push_back({conBlocks[i], conBlocksOrder[i]});
        }
        /**sort array*/
        std::sort(poolEle.begin(), poolEle.end());

        return poolEle;
    }



    /** simulation section*/

    void FlowBlockBase::beforePrepareSim(FlowSimEngine::FLOW_Meta_afterMf simMeta) {
        /**set flow for this element*/
        simMeta._writer->localName = getConCatInheritName();
        getSimEngine()->setSimMeta(simMeta);
        /*** invoke prepare sim in subelement in order*/
        std::vector<FlowBlockBase::sortEle> poolEle = sortSubAndConFbInOrder();
        for (auto& pl: poolEle){
            pl.fb->beforePrepareSim({simMeta._writer->populateSubEle()});
        }
    }




    std::string FBT_to_string(FLOW_BLOCK_TYPE fbt){
        std::string mapper[FLOW_BLOCK_COUNT] = {
                "SEQUENTIAL",
                "PARALLEL_AUTO_SYNC",
                "PARALLEL_NO_SYN",
                "CIF",
                "SIF",
                "CSELIF",
                "CSELSE",
                "ZIF",
                "ZELIF",
                "ZELSE",
                "CWHILE",
                "SWHILE",
                "EXITWHILE",
                "CONDWAIT",
                "CLKWAIT",
                "DO_WHILE",
                "DUMMY_BLOCK"
        };
        assert(fbt < FLOW_BLOCK_COUNT);
        return mapper[fbt];
    }


}