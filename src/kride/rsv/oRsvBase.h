//
// Created by tanawin on 6/4/2025.
//

#ifndef SRC_KRIDE_RSV_RSVBASE_H
#define SRC_KRIDE_RSV_RSVBASE_H

#include"kride/incl.h"
#include"rsvBase.h"

namespace kathryn{

    /////// out of order reservation station
    struct ORSV_BASE: RsvBase{

        Table entries;

        ORSV_BASE(int rsvId, const std::string& rm,
                  RowMeta& userMeta, int addrLength,
                  D_ALL& din, D_IO_RSV& dcIn):
        RsvBase(rsvId, rm, din, dcIn),
        rsvName(rm),
        entries(rm, RsvEntryMeta + userMeta, addrLength){
            assert(addrLength >= 1); /////// if have only one row, why don't use slot
        }

        void buildIssuePtr{
            /////// TODO we must use the sort system
        };

        Oprs findFreeList(int amt){

            /////// integrity check if it exceed, there must be something went wrong
            assert(amt <= entries._amtSize);
            auto freeEntries = entries.buildFindMultiLogic(
                "busy", true, false, amt);
            Oprs indexResults;
            for(Candidate& freeEntry: freeEntries){
                indexResults.emplace_back(*freeEntry.detIdx);
            }
            return indexResults;
        }

        void setEntry(Operable& addr, const Slot& dataSlot){
            ////// augment slot to support the busy value
            entries.assign(dataSlot, addr, true);
        }

        void flow() override{
            buildIssuePtr();
        }

        Operable& buildAkb() override{   ///// do something with it

        }

    };


}

#endif //SRC_KRIDE_RSV_RSVBASE_H
