//
// Created by tanawin on 15/1/2567.
//

#ifndef KATHRYN_RTLSIMRESULT_H
#define KATHRYN_RTLSIMRESULT_H

#include <cassert>
#include "sim/logicRep/valRep.h"

namespace kathryn{

    enum RTL_SIM_ElE_TYPE{
        RSET_HWCOMP,
        RSET_NODE,
        RSET_FLOWBLOCK,
        RSET_MODULE,
        RSET_CNT = 4
    };

    struct RtlSimEleBased{

        RTL_SIM_ElE_TYPE _resultType;

        explicit RtlSimEleBased(RTL_SIM_ElE_TYPE _resultType);
        virtual ~RtlSimEleBased() = default;
    };

    /**
     * It is used to wrap element that used to sim hardware component
     * such as register wire expression
     * */

    struct HwCompSimEle : RtlSimEleBased{
        ValRep* rep    = nullptr;
        ValRep* bufRep = nullptr;

        explicit HwCompSimEle(int bitsz);

    };

    /**
     * It is used to wrap node and collect node statistic
     * */

    struct NodeSimEle : RtlSimEleBased{
        int accessTimes = 0;

        explicit NodeSimEle();
    };

    /**
     * It is used to wrap flow Block and collect node statistic
     * */
     struct FlowBlockSimEle : RtlSimEleBased{
         int accessTimes = 0;

         explicit FlowBlockSimEle();
     };

     /**
      * it is used to wrap module and collect node statistic
      * */
     struct ModuleSimEle

}

#endif //KATHRYN_RTLSIMRESULT_H
