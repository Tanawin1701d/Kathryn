//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H
#define KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H

#include "kathryn.h"
#include "stageStruct.h"
using namespace std;

#define sValid first
#define sIdx   second

namespace kathryn::o3{
    typedef std::pair<Operable&, Operable&> SearchResult;
    ///////// valid / index binary
    inline std::pair<Operable&,Operable&>
    searchIdx(Table& table,int value,
              bool isBegin, BroadCast& bc,
              bool checkNext){

        auto [iw, binIdx] =
        table.doReducBinIdx([&](
             WireSlot& lhs, Operable* lidx,
             WireSlot& rhs, Operable* ridx) -> Operable&{

            WireSlot& sidedSlot = isBegin ? lhs : rhs;
            Operable* checkBusy = &sidedSlot(busy);
            if (checkNext){
                auto& isSpec = sidedSlot(spec);
                auto& isKilled = bc.checkIsKill(sidedSlot(specTag));
                checkBusy = &(sidedSlot(busy) & (~isSpec | isKilled));
            }


            if (isBegin){
                return (*checkBusy) == value; /// select left we want begin
            }
            return (*checkBusy) != value; /// select left if right is not really correct

        });

        return{iw(busy) == value, binIdx};
    }


}


#endif //KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H