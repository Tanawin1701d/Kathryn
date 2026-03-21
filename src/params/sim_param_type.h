//
// Created by tanawin on 7/6/2024.
//

#ifndef SIMPARAMTYPE_H
#define SIMPARAMTYPE_H


namespace kathryn{

    enum MODULE_VCD_REC_POL{  ///// module vcd record policy
        MDE_REC_ONLY_USER = 0,
        MDE_REC_ONLY_INTERNAL = 1,
        MDE_REC_BOTH = 2,
        MDE_REC_SKIP = 3
    };

    enum MODULE_FLOW_PERF_PARAM{
        MFP_ON = 0,
        MFP_OFF = 1
    };


}

#endif //SIMPARAMTYPE_H
