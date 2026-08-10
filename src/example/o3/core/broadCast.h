//
// Created by tanawin on 6/10/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H
#define KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H

#include "kathryn.h"
#include "parameter.h"


namespace kathryn::o3{

    struct BroadCast{                                ///MD TAG
        mWire(mis, 1);                               ///CTRL_HWD TAG
        mWire(fixTag, SPECTAG_LEN);                  ///CTRL_HWD TAG
        mWire(suc, 1);                               ///CTRL_HWD TAG
        mWire(sucTag, SPECTAG_LEN);                  ///CTRL_HWD TAG
        opr& isBrMissPred(){ return mis;}            ///CTRL_HC TAG
        opr& isBrSuccPred(){ return suc;}            ///CTRL_HC TAG
        opr& checkIsKill(opr& specIdx){              ///CTRL_HC TAG
            return mis & ( (fixTag&specIdx) != 0);   ///CTRL_CL TAG
        }
        opr& checkIsSuc (opr& specIdx){         ///CTRL_HC TAG
            return suc & (sucTag == specIdx);   ///CTRL_CL TAG
        }
        opr& checkIsSuc(Slot& src){             ///DATA_HC TAG

            opr& specOpr = *src.hwFieldAt(spec)._opr;       ///CTRL_DT TAG
            opr& specIdx  = *src.hwFieldAt(specTag)._opr;   ///CTRL_DT TAG
            return suc & specOpr & (sucTag == specIdx);     ///CTRL_CL TAG
        }
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_BROADCAST_H
