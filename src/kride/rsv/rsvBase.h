//
// Created by tanawin on 6/4/2025.
//

#ifndef SRC_KRIDE_RSV_RSVBASE_H
#define SRC_KRIDE_RSV_RSVBASE_H

#include"kride/incl.h"

namespace kathryn{

    /////// out of order reservation station
    struct ORSV_BASE: Module{
        Table entries;

        ORSV_BASE(const std::string& tableName,
                  RowMeta& userMeta, int addrLength):
        entries(tableName, OORsvEntry + userMeta, addrLength){
            assert(addrLength >= 1); /////// if have only one row, why don't use slot
        }

        void flow() override{

        }

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

        void setEntry(Operable& addr, Slot dataSlot){

        }

    };


}

#endif //SRC_KRIDE_RSV_RSVBASE_H
