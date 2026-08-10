//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H
#define KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H

#include "stageStruct.h"

using namespace std;

#define sValid first
#define sIdx   second

namespace kathryn::o3{
    typedef std::pair<Operable&, Operable&> SearchResult;   ///HLH SHARED_COMP
    ///////// valid / index binary
    inline std::pair<Operable&,Operable&>    ///CTRL_HC+DATA_HC SHARED_COMP
    searchIdx(Table& table,int value,        ///DATA_HC SHARED_COMP
              bool isBegin, BroadCast& bc,   ///CTRL_HC SHARED_COMP
              bool checkNext){               ///HLH SHARED_COMP

        auto [iw, binIdx] =                                              ///HLH SHARED_COMP
        table.doReducBinIdx([&](                                         ///HLH SHARED_COMP
             WireSlot& lhs, Operable* lidx,                              ///CTRL_HC+DATA_HC SHARED_COMP
             WireSlot& rhs, Operable* ridx) -> Operable&{                ///CTRL_HC+DATA_HC SHARED_COMP

            WireSlot& sidedSlot = isBegin ? lhs : rhs;                   ///DATA_CL SHARED_COMP
            Operable* checkBusy = &sidedSlot(busy);                      ///CTRL_DT SHARED_COMP
            if (checkNext){                                              ///HLH SHARED_COMP
                auto& isSpec = sidedSlot(spec);                          ///CTRL_DT SHARED_COMP
                auto& isKilled = bc.checkIsKill(sidedSlot(specTag));     ///CTRL_CL SHARED_COMP
                checkBusy = &(sidedSlot(busy) & ~(isSpec & isKilled));   ///CTRL_CL SHARED_COMP
            }

            if (isBegin){   ///HLH SHARED_COMP
                return (*checkBusy) == value; /// select left we want begin   ///CTRL_CL SHARED_COMP
            }
            return (*checkBusy) != value; /// select left if right is not really correct   ///CTRL_CL SHARED_COMP
        });

        if (checkNext){   ///HLH SHARED_COMP
            return {(iw(busy) && ~(iw(spec) & bc.checkIsKill(iw(specTag)))) == value, binIdx};   ///CTRL_HC+DATA_HC SHARED_COMP
        }
            return{iw(busy) == value, binIdx};   ///CTRL_HC+DATA_HC SHARED_COMP

    }


}


#endif //KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H
