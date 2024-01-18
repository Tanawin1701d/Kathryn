//
// Created by tanawin on 15/1/2567.
//


#include "rtlSimResult.h"


namespace kathryn{


    RtlSimEleBased::RtlSimEleBased(RTL_SIM_ElE_TYPE resultType):
    _resultType(resultType)
    {
        assert(resultType < RSET_CNT);
    }


    /**
     * HW COMP sim result
     * */
    HwCompSimEle::HwCompSimEle(int bitsz):
        RtlSimEleBased(RSET_HWCOMP),
        rep   (new ValRep(bitsz)),
        bufRep(new ValRep(bitsz))
    {
        assert(bitsz > 0);
    }

    /**
     * NODE SIM ELE
     * */
    NodeSimEle::NodeSimEle():
        RtlSimEleBased(RSET_NODE),
        accessTimes(0)
    {}

    /**
     * Flowblock SIM ELE
     * */
    FlowBlockSimEle::FlowBlockSimEle():
            RtlSimEleBased(RSET_FLOWBLOCK),
            accessTimes(0)
    {}

    /**
     * Module SIM ELE
     * */


}