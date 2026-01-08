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
        mWire(fixTag, SPECTAG_LEN);
        mWire(suc, 1);
        mWire(misTag, SPECTAG_LEN);
        mWire(sucTag, SPECTAG_LEN);
        opr& isBrMissPred(){ return mis;}
        opr& isBrSuccPred(){ return suc;}
        opr& checkIsKill(opr& specIdx){
            return mis & ( (fixTag&specIdx) != 0);
        }
        opr& checkIsSuc (opr& specIdx){
            return suc & (sucTag == specIdx);
        }

        opr& checkIsSuc(Slot& src){

            opr& specOpr = *src.hwFieldAt(spec)._opr;
            opr& specIdx  = *src.hwFieldAt(specTag)._opr;
            return suc & specOpr & (sucTag == specIdx);
        }

    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H
