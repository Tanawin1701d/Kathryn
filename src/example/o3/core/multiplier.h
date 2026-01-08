//
// Created by tanawin on 11/12/25.
//
#ifndef KATHRYN_MULTIPLIER_H
#define KATHRYN_MULTIPLIER_H

#include "kathryn.h"
#include "parameter.h"

namespace kathryn::o3{

    inline opr& multiplier(RegSlot& src){

        opr& srcAU = src(phyIdx_1).uext(2*DATA_LEN);
        opr& srcAS = src(phyIdx_1).sext(2*DATA_LEN);

        opr& srcBU = src(phyIdx_2).uext(2*DATA_LEN);
        opr& srcBS = src(phyIdx_2).sext(2*DATA_LEN);

        opr& res_uu = srcAU * srcBU;
        opr& res_us = srcAU * srcBS;
        opr& res_su = srcAS * srcBU;
        opr& res_ss = srcAS * srcBS;

        opr& res = mux( g(src(md_req_in_signed_1), src(md_req_in_signed_2)),
                        {&res_uu, ///// 0 0
                         &res_us, ///// 0 1
                         &res_su, ///// 1 0
                         &res_ss} ///// 1 1
                    );

        return mux(src(md_req_out_sel)(0),
                   res.sl(DATA_LEN, 2 * DATA_LEN),
                   res.sl(0, DATA_LEN)
                   );
    }
}

#endif //KATHRYN_MULTIPLIER_H