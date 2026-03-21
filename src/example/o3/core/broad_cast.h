//
// Created by tanawin on 6/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H
#define KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H

#include "kathryn.h"
#include "parameter.h"


namespace kathryn::o3{

    struct BroadCast{
        m_wire(mis, 1);
        m_wire(fix_tag, SPECTAG_LEN);
        m_wire(suc, 1);
        m_wire(suc_tag, SPECTAG_LEN);
        opr& is_br_miss_pred(){ return mis;}
        opr& is_br_succ_pred(){ return suc;}
        opr& check_is_kill(opr& spec_idx){
            return mis & ( (fix_tag&spec_idx) != 0);
        }
        opr& check_is_suc (opr& spec_idx){
            return suc & (suc_tag == spec_idx);
        }
        opr& check_is_suc(Slot& src){

            opr& spec_opr = *src.hw_field_at(spec)._opr;
            opr& spec_idx  = *src.hw_field_at(spec_tag)._opr;
            return suc & spec_opr & (suc_tag == spec_idx);
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H
