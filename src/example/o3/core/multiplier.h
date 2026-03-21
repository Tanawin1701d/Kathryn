//
// Created by tanawin on 11/12/25.
//
#ifndef KATHRYN_MULTIPLIER_H
#define KATHRYN_MULTIPLIER_H

#include "parameter.h"

namespace kathryn::o3{

    inline opr& multiplier(RegSlot& src){

        opr& src_au = src(phyIdx_1).uext(2*DATA_LEN);
        opr& src_as = src(phyIdx_1).sext(2*DATA_LEN);

        opr& src_bu = src(phyIdx_2).uext(2*DATA_LEN);
        opr& src_bs = src(phyIdx_2).sext(2*DATA_LEN);

        opr& res_uu = src_au * src_bu;
        opr& res_us = src_au * src_bs;
        opr& res_su = src_as * src_bu;
        opr& res_ss = src_as * src_bs;

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