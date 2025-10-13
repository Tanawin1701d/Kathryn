//
// Created by tanawin on 4/10/25.
//
#include "stageStruct.h"

#include "rsv.h"


namespace kathryn::o3{

    //////// bypass pool

    void ByPassPool::doByPass(ByPass& bp){
        bp.valid = 1;
        //////// do override all over rsv
        for (RsvBase* rsv : _rsvs){
            rsv->onBypass(bp);
        }
    }

}
