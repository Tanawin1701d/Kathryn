//
// Created by tanawin on 7/2/2025.
//

#ifndef SRC_CAROLYNE_MODEL_BASE_FETCH_FETCHSTAGEBASE_H
#define SRC_CAROLYNE_MODEL_BASE_FETCH_FETCHSTAGEBASE_H

#include "lib/hw/popper/popMod.h"
#include "carolyne/arch/base/march/repo/repo.h"
#include "carolyne/model/base/coreArgs.h"

namespace kathryn::carolyne{


    /////// fetch unit model base
    struct FetchUM_Base: BlkPop{

        FetchUTM_Base& _fetchMeta;

        explicit FetchUM_Base(FunArgs funArgs, FetchUTM_Base& fetchMeta):
        BlkPop(funArgs, FETCH_BLOCK_NAME),
        _fetchMeta(fetchMeta){}

        /*** blk pop-loop will handle how fetch block fetch each instruction**/

        void blk_popLoop(FlowBlockBase* kathrynBlock) override{
            /////// interrupt signal
            Operable& mispred = getIntrSignal(INTR_MISPRED);
            intrReset(mispred);
            intrStart(mispred);
            /////// collect all related tunnel
            PopHST& ctrlTun   = getHST(TN_PC); ///// slave
            PopHST& memTun    = getHST(TN_FETMEM); ///// master
            PopHST& decodeTun = getHST(TN_FETDEC);

            ////// procedure
            ////// 1.memory port trigger by return the data
            ////// 2.composed the data to decoder
            ////// 3.acktheSendOfcurrentFetchrequest new memory addr

            /////// memory port trigger and send to decoder
            cdowhile(!decodeTun.isAckSend()){
                zif(memTun.isReqRet()){
                    decodeTun.reqSend(memTun.getReqRet());
                }
            }
            //////// interrupt signal will reset it
            Operable& decodeAcked      = observe({&decodeTun.isAckSend()}, nullptr);
            Operable& readyToFetchNext = (ctrlTun.isReqSend() && decodeAcked);
            ////// decoder trigger and req new pc
            cdowhile( (!readyToFetchNext) || (!memTun.isAckSend()) ){ markJoinMaster
                zif(readyToFetchNext){
                    memTun.reqSend(ctrlTun.getReqSend()({SLOT_F_PC}));
                }
            }
        }





    };

}


#endif //SRC_CAROLYNE_MODEL_BASE_FETCH_FETCHSTAGEBASE_H
