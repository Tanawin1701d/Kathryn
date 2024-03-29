//
// Created by tanawin on 2/12/2566.
//

#include "flowBlock_Base.h"

#include <utility>
#include "model/controller/controller.h"
#include "model/flowBlock/seq/seq.h"
#include "model/flowBlock/par/par.h"

namespace kathryn{

    int nextFbIdx = 0;


    FlowBlockBase::FlowBlockBase(FLOW_BLOCK_TYPE type, FB_CTRL_COM_META fbCtrlComMeta):
            FlowIdentifiable(FBT_to_string(type)),
            FlowSimEngine(),
            /** flow element*/
            _type(type),
            ctrl(getControllerPtr()),
            lazyDeletedRequired(false),
            _fbId(nextFbIdx++),
            /** controller communication policy*/
            _fbCtrlComMeta(std::move(fbCtrlComMeta)),
            /** exit management*/
            _areThereForceExit(false),
            _forceExitNode(nullptr)
    {}

    FlowBlockBase::~FlowBlockBase(){
        for (auto basicNode : _basicNodes){
            delete basicNode;
        }
        for (auto sub_fb: _subBlocks){
            delete sub_fb;
        }
        for (auto con_fb: _conBlocks){
            delete con_fb;
        }
        for (auto abandon_fb:_abandonedBlocks){
            delete abandon_fb;
        }
        delete _forceExitNode;
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

    void FlowBlockBase::buildSubHwComponent(){

        for (auto subBlockPtr: _subBlocks){
            assert(subBlockPtr != nullptr);
            subBlockPtr->buildHwComponent();
        }

        for (auto conBlockPtr: _conBlocks){
            assert(conBlockPtr != nullptr);
            conBlockPtr->buildHwComponent();
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
            _areThereForceExit |= (nw->getForceExitNode() != nullptr);
        }
        if (_areThereForceExit){
            _forceExitNode = new PseudoNode(1);
            for (auto nw : nws){
                if (nw->getForceExitNode() != nullptr){
                    _forceExitNode->addDependNode(nw->getForceExitNode());
                }
            }
            _forceExitNode->setDependStateJoinOp(BITWISE_OR);
            _forceExitNode->assign();

        }
    }

    std::vector<FlowBlockBase::sortEle> FlowBlockBase::sortSubAndConFbInOrder() {

        std::vector<sortEle> poolEle;
        /***pool sub block in to the array*/
        for(int i = 0; i < _subBlocks.size(); i++){
            poolEle.push_back({_subBlocks[i], _subBlocksOrder[i]});
        }
        for(int i = 0; i < _conBlocks.size(); i++){
            poolEle.push_back({_conBlocks[i], _conBlocksOrder[i]});
        }
        /**sort array*/
        std::sort(poolEle.begin(), poolEle.end());

        return poolEle;
    }



    /** simulation section*/

    void FlowBlockBase::beforePrepareSim(FLOW_Meta_afterMf simMeta) {
        /**set flow for this element*/
        FlowSimEngine::beforePrepareSim(simMeta);
        simMeta._writer->localName = getGlobalName();
        /*** invoke prepare sim in subelement in order*/
        std::vector<FlowBlockBase::sortEle> poolEle = sortSubAndConFbInOrder();
        for (auto& pl: poolEle){
            ////// recurent build track element to system
            pl.fb->beforePrepareSim({simMeta._writer->populateSubEle()});
        }
    }




    std::string FBT_to_string(FLOW_BLOCK_TYPE fbt){
        std::string mapper[FLOW_BLOCK_COUNT] = {
                "SEQUENTIAL", /** seq to par_no_syn do not reorder it due to controller pattern checking*/
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
                "PIPE_SENDER",
                "PIPE_RECIEVER",
                "PIPE_BLOCK",
                "PIPE_WRAPPER",
                "DUMMY_BLOCK"
        };
        assert(fbt < FLOW_BLOCK_COUNT);
        return mapper[fbt];
    }


}