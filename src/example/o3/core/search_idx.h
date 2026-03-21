//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H
#define KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H

#include "stage_struct.h"

using namespace std;

#define s_valid first
#define s_idx   second

namespace kathryn::o3{
    typedef std::pair<Operable&, Operable&> SearchResult;
    ///////// valid / index binary
    inline std::pair<Operable&,Operable&>
    search_idx(Table& table,int value,
              bool is_begin, BroadCast& bc,
              bool check_next){

        auto [iw, bin_idx] =
        table.do_reduc_bin_idx([&](
             WireSlot& lhs, Operable* lidx,
             WireSlot& rhs, Operable* ridx) -> Operable&{

            WireSlot& sided_slot = is_begin ? lhs : rhs;
            Operable* check_busy = &sided_slot(busy);
            if (check_next){
                auto& is_spec = sided_slot(spec);
                auto& is_killed = bc.check_is_kill(sided_slot(spec_tag));
                check_busy = &(sided_slot(busy) & ~(is_spec & is_killed));
            }

            if (is_begin){
                return (*check_busy) == value; /// select left we want begin
            }
            return (*check_busy) != value; /// select left if right is not really correct
        });

        if (check_next){
            return {(iw(busy) && ~(iw(spec) & bc.check_is_kill(iw(spec_tag)))) == value, bin_idx};
        }
            return{iw(busy) == value, bin_idx};

    }


}


#endif //KATHRYN_SRC_EXAMPLE_O3_SEARCH_IDX_H