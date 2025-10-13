//
// Created by tanawin on 6/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H
#define KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H

#include "kathryn.h"
#include "parameter.h"


namespace kathryn::o3{

    struct BroadCast{
        mWire(mis, 1);
        mWire(suc, 1);
        mWire(misTag, SPECTAG_LEN);
        mWire(sucTag, SPECTAG_LEN);
        opr& isBrMissPred(){ return mis;}
        opr& isBrSuccPred(){ return suc;}
        opr& checkIsKill(opr& specIdx){
            return mis & (misTag == specIdx);
        }
        opr& checkIsSuc (opr& specIdx){
            return suc & (sucTag == specIdx);
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H
