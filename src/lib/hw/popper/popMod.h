//
// Created by tanawin on 26/1/2025.
//

#ifndef src_lib_hw_popper_POPMOD_H
#define src_lib_hw_popper_POPMOD_H
#include "popInterrupt.h"
#include "popTunnel/popHST.h"
#include "popTunnel/popBCT.h"
#include "popTunnel/popTunnelMng.h"
#include "model/controller/controller.h"
#include "lib/hw/observe/observe.h"

namespace kathryn{


    struct PopMod: Module, PopIntrable, PopTunnelable{

        std::vector<Operable*> trackedRstSignal;
        std::vector<Operable*> trackedIntrSignal;
        FlowBlockBase* popLoopParBlock = nullptr;

        explicit PopMod(PopIntrMng* intrMng, PopTunnelMng* tunnelMng):
            PopIntrable(intrMng), PopTunnelable(tunnelMng){}

        /** main control flow*/

        void flow() override{
            cwhile(true){ par{
                popLoopParBlock = kathrynBlock;
                blk_popLoop(kathrynBlock);
            }}
            blk_userFlow();
        }

        virtual void blk_popLoop (FlowBlockBase* kathrynBlock) {}
        virtual void blk_userFlow() {}

        /** if all signal is one (unnecessary at the same cycle)
         * then the blk_popLoop will do the reloop
         * all sig in sigs size must be one
         * */
        void blk_trackAndReLoop(const std::vector<Operable*>& sigs) const{
            mfAssert(popLoopParBlock != nullptr, "cannot dectect poploop block for track and reloop operation it caused from trackAndReLoop mis placed");
            Operable& obsResult = observe(sigs, popLoopParBlock->genIntSumSignal(true, INT_RESET));
            /** we will use scWait but we must use markJoinMaster*/
            for(auto kathrynBlock = new FlowBlockCondWait(obsResult);
                     kathrynBlock->doPrePostFunction();
                     kathrynBlock->step()){
                markJoinMaster
            };
        }

        void blk_reLoopAt(Operable& cond) const{
            for(auto kathrynBlock = new FlowBlockCondWait(cond);
                kathrynBlock->doPrePostFunction();
                kathrynBlock->step()){
                markJoinMaster
            };
        }

    };

}

#endif //src_lib_hw_popper_POPMOD_H
