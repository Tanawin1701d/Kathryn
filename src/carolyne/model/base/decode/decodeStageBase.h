//
// Created by tanawin on 13/2/2025.
//

#ifndef src_carolyne_model_base_decode_DECODESTAGEBASE_H
#define src_carolyne_model_base_decode_DECODESTAGEBASE_H

#include "decodeSlotAnal.h"
#include "lib/hw/popper/popMod.h"
#include "carolyne/arch/base/march/repo/repo.h"
#include "carolyne/arch/base/isa/repo/repo.h"
#include "carolyne/model/base/coreArgs.h"

namespace kathryn::carolyne{



    struct DecodeUM_Base: BlkPop{
        IsaBaseRepo&       _isaBaseRepo;
        DecodeSlotAnalyzer _decAnal;
        mReg(markColdStart, 1);


        explicit DecodeUM_Base(FunArgs funArgs, IsaBaseRepo& isaBaseRepo):
        BlkPop(funArgs, DECODE_BLOCK_NAME),
        _isaBaseRepo(isaBaseRepo),
        _decAnal(isaBaseRepo){
            /////// iterate all uopType and make DecAnal check the opr formation
            _decAnal.analIsa();
            _decAnal.buildDecoderSlot();

        }

        void blk_popLoop(FlowBlockBase* kathrynBlock) override{
            /////// interrupt signal
            Operable& mispred = getIntrSignal(INTR_MISPRED);
            intrRstAndStart(mispred);
            /////// collect all related tunnel
            PopHST& fetchTun = getHST(TN_FETDEC);
            PopHST& allocTun = getHST(TN_DECALLOC);
            ////// procedure (in one cycle)
            ////// 1. decodeBlock trigs allocUnit with decoded data
            ////// 2. fetch block trigs next decodeBlock

            ////// 1. decodeBlock trigs allocUnit with decoded data
            cif(~markColdStart){
                allocTun.blk_reqAndWait4Ack(_decAnal.getDecoderSlot());
            }
            ////// 2. fetch block trigs next decodeBlock
            Operable& allocAcked        = observe({&allocTun.isAckSend()}, {&mispred, &fetchTun.isAckSend()});
            Operable& readyToDecodeNext = allocAcked | markColdStart;
            cdowhile(!fetchTun.isAckSend()){
                zif(readyToDecodeNext && fetchTun.isReqSend()){
                    fetchTun.ackSend();
                    _decAnal.blk_decode(fetchTun.getReqSend());
                }
            }


        }


    };

}

#endif //src_carolyne_model_base_decode_DECODESTAGEBASE_H
