//
// Created by tanawin on 24/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_RSV_H
#define KATHRYN_SRC_EXAMPLE_O3_RSV_H


#include "kathryn.h"
#include "slotParam.h"
#include "stageStruct.h"

namespace kathryn::o3{

    struct RsvBase{
        SlotMeta _meta;
        Table    _table;

        RsvBase(SlotMeta meta, int amtRow):
            _meta(meta), _table(meta, amtRow){}

        virtual ~RsvBase() = default;

        virtual RegSlot buildIssue(SyncMeta& syncMeta, BroadCast& bc) = 0;

        Operable& slotReady(WireSlot& iw){
            return iw(busy) && iw(rsValid_1) && iw(rsValid_2);
        }

        void tryOwSpecBit(RegSlot& resultRegSlot, WireSlot& iw, BroadCast& bc){
            ///////// we have to override the spec bit if it is on the fly
            auto& isSpec    = iw(spec);
            auto& specTagIdx= iw(specTag);
            //// send data

            zif ( isSpec && bc.checkIsSuc(specTagIdx)){
                resultRegSlot(spec) <<= 0;
            }

        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_RSV_H
