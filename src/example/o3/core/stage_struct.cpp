//
// Created by tanawin on 4/10/25.
//
#include "stage_struct.h"

#include "rsv.h"
#include "rsvs.h"


namespace kathryn::o3{

    //////// bypass pool

    void ByPassPool::do_by_pass(ByPass& bp){
        bp.valid = 1;
        //////// do override all over rsv
        for (RsvBase* rsv : _rsvs->rsvs){
            rsv->on_bypass(bp);
        }
    }

}
