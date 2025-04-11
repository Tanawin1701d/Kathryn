//
// Created by tanawin on 8/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_RSV_RSVBASE_H
#define KATHRYN_SRC_KRIDE_RSV_RSVBASE_H

#include <utility>

#include "kride/incl.h"

namespace kathryn{

    struct RsvBase: Module{
        const int         rsvId = -1;
        const std::string rsvName;
        D_ALL&            d;
        D_IO_RSV&         dc;


        RsvBase(int rsvIdIn, std::string  rm,
                D_ALL& din , D_IO_RSV& dcIn):
        rsvId(rsvIdIn), rsvName(std::move(rm)),
        d(din)        , dc(dcIn){}
        virtual Operable& buildAkb() = 0;
    };

}

#endif //KATHRYN_SRC_KRIDE_RSV_RSVBASE_H
