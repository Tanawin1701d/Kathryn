//
// Created by tanawin on 11/12/25.
//
#ifndef KATHRYN_MULTIPLIER_H
#define KATHRYN_MULTIPLIER_H

#include "parameter.h"

namespace kathryn::o3{

    inline opr& multiplier(RegSlot& src){   ///DATA_HC EXEC_MUL

        opr& srcAU = src(phyIdx_1).uext(2*DATA_LEN);   ///DATA_CL EXEC_MUL
        opr& srcAS = src(phyIdx_1).sext(2*DATA_LEN);   ///DATA_CL EXEC_MUL

        opr& srcBU = src(phyIdx_2).uext(2*DATA_LEN);   ///DATA_CL EXEC_MUL
        opr& srcBS = src(phyIdx_2).sext(2*DATA_LEN);   ///DATA_CL EXEC_MUL

        opr& res_uu = srcAU * srcBU;   ///DATA_CL EXEC_MUL
        opr& res_us = srcAU * srcBS;   ///DATA_CL EXEC_MUL
        opr& res_su = srcAS * srcBU;   ///DATA_CL EXEC_MUL
        opr& res_ss = srcAS * srcBS;   ///DATA_CL EXEC_MUL

        opr& res = mux( g(src(md_req_in_signed_1), src(md_req_in_signed_2)),   ///DATA_CL EXEC_MUL
                        {&res_uu, ///// 0 0 ///DATA_CL EXEC_MUL
                         &res_us, ///// 0 1 ///DATA_CL EXEC_MUL
                         &res_su, ///// 1 0 ///DATA_CL EXEC_MUL
                         &res_ss} ///// 1 1 ///DATA_CL EXEC_MUL
                    );

        return mux(src(md_req_out_sel)(0),   ///DATA_CL EXEC_MUL
                   res.sl(DATA_LEN, 2 * DATA_LEN), ///DATA_CL EXEC_MUL
                   res.sl(0, DATA_LEN) ///DATA_CL EXEC_MUL
                   );
    }
}

#endif //KATHRYN_MULTIPLIER_H
