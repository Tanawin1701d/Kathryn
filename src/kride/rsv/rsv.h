//
// Created by tanawin on 8/4/2025.
//

#ifndef KATHRYN_KRIDE_RSV_RSV_H
#define KATHRYN_KRIDE_RSV_RSV_H

#include "kride/incl.h"
#include "oRsvBase.h"
#include "iRsvBase.h"

namespace kathryn{

    /////////// out of order reservation station

    struct  IntRsv: ORSV_BASE{
        IntRsv(D_ALL& din, D_IO_RSV& dcIn):
            ORSV_BASE(RS_ENT_ALU, RSV_INT,
                      intREM ,RSV_INT_SEL,
                      din, dcIn){}
    };

    struct MulRsv: ORSV_BASE{
        MulRsv(D_ALL& din, D_IO_RSV& dcIn):
            ORSV_BASE(RS_ENT_MUL, RSV_MUL,
                      mulREM, RSV_MUL_SEL,
                      din, dcIn){}
    };

    struct BrRsv: IRSV_BASE{
        BrRsv(D_ALL& din, D_IO_RSV& dcIn):
            IRSV_BASE(RS_ENT_BRANCH, RSV_BR,
                      brREM, RSV_BR_SEL,
                      din, dcIn){};
    };

    struct LdStRsv: IRSV_BASE{
        LdStRsv(D_ALL& din, D_IO_RSV& dcIn):
            IRSV_BASE(RS_ENT_LDST, RSV_LDST,
                      ldstREM, RSV_LDST_SEL,
                      din, dcIn){};
    };

}

#endif //KATHRYN_KRIDE_RSV_RSV_H
