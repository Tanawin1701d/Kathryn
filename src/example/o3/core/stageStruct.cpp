//
// Created by tanawin on 4/10/25.
//
#include "stageStruct.h"

#include "rsv.h"
#include "rsvs.h"


namespace kathryn::o3{

    //////// bypass pool

    void ByPassPool::doByPass(ByPass& bp){
        bp.valid = 1;
        //////// do override all over rsv
        for (RsvBase* rsv : _rsvs->rsvs){
            rsv->onBypass(bp);
        }
    }

}
