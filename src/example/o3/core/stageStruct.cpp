//
// Created by tanawin on 4/10/25.
//
#include "stageStruct.h"

#include "rsv.h"
#include "rsvs.h"


namespace kathryn::o3{

    //////// bypass pool

    void ByPassPool::doByPass(ByPass& bp){   ///CTRL_HC+DATA_HC CORE
        bp.valid = 1;                        ///CTRL_DT CORE
        //////// do override all over rsv
        for (RsvBase* rsv : _rsvs->rsvs){    ///HLH CORE
            rsv->onBypass(bp);               ///CTRL_HC+DATA_HC CORE
        }
    }

}
