//
// Created by tanawin on 5/3/2025.
//

#ifndef SRC_MODEL_FLOWBLOCK_PIPELINE_PIPETRAN_H
#define SRC_MODEL_FLOWBLOCK_PIPELINE_PIPETRAN_H

////   SRC_MODEL_FLOWBLOCK_PIPELINE_PIPETRAN_H


#include "model/flowBlock/abstract/flowBlock_Base.h"
#include "model/flowBlock/abstract/loopStMacro.h"
#include "model/flowBlock/abstract/spReg/stateReg.h"
#include "model/flowBlock/abstract/nodes/node.h"
#include "model/flowBlock/abstract/nodes/stateNode.h"


#define pipTran(pipeName)           for(auto kathrynBlock = new FlowBlockPipeTran (pipeName); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define pipTranWhen(pipeName, cond) for(auto kathrynBlock = new FlowBlockPipeTran (pipeName, &cond); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define pipMTran                    for(auto kathrynBlock = new FlowBlockPipeTran(); kathrynBlock->doPrePostFunction(); kathrynBlock->step())
#define tranTo  (pipeName)          kathrynBlock->addTranMeta(pipeName, nullptr);
#define tranToWC(pipeName, cond)    kathrynBlock->addTranMeta(pipeName, &(cond));

/////// sync will use with the offer
/// offer su
#define pipSyncs(offerVec)          kathrynBlock->addSyncMeta(offerVecs);
#define pipSync(offer)              kathrynBlock->addSyncMeta(offer, nullptr);
#define pipSyncWhen(offer, cond)    kathrynBlock->addSyncMeta(offer, &cond);
//////// pipe sync with custom choice
#define pipSyncWCC(offer, choice)       kathrynBlock->addSyncMeta(offer, choice, nullptr);
#define pipSyncMan(offer, choice, cond) kathrynBlock->addSyncMeta(offer, choice, &cond);




namespace kathryn{

    const char PIPE_TARGET_UNNAME[] = "PIPE_TARGET_UNNAMED";


    //////////// ACTIVATE SIGNAL --- is a signal that declares for transaction ready to transfer but
    //////////// however, the transaction must co operate with transaction stateMatchine state machine;
    ///
    //////////// READY SIGNAL ---- is the signal that determine both tran's state machine and activation signal is
    //////////// ready to change their state.


    struct PipeTranMeta{
        ///// src signal
        std::string _targetPipeName;
        Operable*   _userAddCond    = nullptr;
        ///// post signal
        expression* _activateSignal    = nullptr;

        void        createActivateSignal(); ///// just create for predefine the signal
        void        assignActivateSignal(Operable* rstSignal) const;
        Operable* addLogic(Operable* src0, Operable* src1, LOGIC_OP lop) const;
    };
    struct PipeSyncMeta{
        ///// src signal
        const std::string& _masterOffer;
        const std::string& _choiceName; /// typically we use pipe name
        Operable*          _userReqOfferIf    = nullptr; /// if not it is automatically activate
        ////// post signal
        expression*        _intendOfferSignal = nullptr;
        Operable*          _acceptSignal      = nullptr;
        void        createPredefSignal(); //// create predefine signal
        void        assignOfferSignal();  ///// offer from the offerBase
        void        assignAcceptSignal(Operable* acceptSig);    ///// ack   return to the offer base
        void        notifyOfferPoolerAcceptSig(); //// connect to the pooler to notify the system
        Operable*   addLogic(Operable* src0, Operable* src1, LOGIC_OP lop) const;

    };

    class FlowBlockPipeBase;
    class FlowBlockPipeTran: public FlowBlockBase, public LoopStMacro{

    protected:
        FlowBlockPipeBase*        _hostPipe = nullptr; ///// hostPipe will be retrieved when the block is attack
        std::vector<PipeTranMeta> _pipeTranMetas;
        std::vector<PipeSyncMeta> _pipeSyncMetas;
        LOGIC_OP                  _tranPolicy = BITWISE_OR;

        Operable*                 _offerReadySignal      = nullptr;
        Operable*                 _targetPipeReadySignal = nullptr;
        ////// the signal that indicate that the pipeTranblock is ready to go
        Operable*                 _activateSignal = nullptr;
        /////// POST MODEL // MF PROCESS
        //////////// node
        PseudoNode*               condNode       = nullptr;
        PseudoNode*               exitNode       = nullptr;
        StateNode*                basicStNode    = nullptr;
        NodeWrap*                 resultNodeWrap = nullptr;
        ///////// proxy signal
        expression*               basicStNodeExitExpr = nullptr;

    public:
        explicit FlowBlockPipeTran();
        explicit FlowBlockPipeTran(std::string  targetPipeName);
                 FlowBlockPipeTran(const std::string& targetPipeName, Operable* customCond);
        ~FlowBlockPipeTran() override;
        /** for controller add the local element to this sub block*/
        void addElementInFlowBlock(Node* node) override;
        void addSubFlowBlock(FlowBlockBase* subBlock) override;
        void addConFlowBlock(FlowBlockBase* conBlock) override;
        void addAbandonFlowBlock(FlowBlockBase* abandonBlock) override;
        NodeWrap* sumarizeBlock() override;
        /**set activate bias usually used in init Pipe */
        void addTranMeta(const std::string& targetPipeName,
                         Operable* userCond = nullptr);
        /** sync component*/
        void addSyncMeta(const std::string& offerName, const std::string& choiceName, Operable* activateIf);
        void addSyncMeta(const std::string& offerName, Operable* activateIf);

        void addSyncMetas(const std::vector<std::string>& tranMetas);
        void assignTargetPipeActivateCond(); ///////// connect to pooler to get each signal target pipe is ready to transfer
        void assignOfferActivateCond();

        [[nodiscard]] Operable*  joinTargetPipeActivateCond() const;
        [[nodiscard]] Operable*  joinTargetOfferActivateCond() const;
        [[nodiscard]] Operable*  getReadySignal    (const std::string& pipeTarget) const;
        [[nodiscard]] std::vector<std::string> getTranTargetNames() const;
        /** get host pipe from controller*/
        FlowBlockPipeBase* getHostPipeFromCtrl();

        /** on this block is start interact to controller*/
        void onAttachBlock() override;
        /** on leave this block*/
        void onDetachBlock() override;
        /** for module to build hardware component*/
        void buildHwComponent() override;
        /** get describe*/
        void addMdLog(MdLogVal* mdLogVal) override;
        /** Loop macro to notice position of system*/
        void doPreFunction() override;
        void doPostFunction() override;
    };


}




#endif //SRC_MODEL_FLOWBLOCK_PIPELINE_PIPETRAN_H
