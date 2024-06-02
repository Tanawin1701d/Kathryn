//
// Created by tanawin on 5/5/2567.
//

#ifndef KATHRYN_SIMPARAM_H
#define KATHRYN_SIMPARAM_H


namespace kathryn{

    enum MODULE_VCD_REC_POL{  ///// module vcd record policy
        MDE_REC_ONLY_USER,
        MDE_REC_ONLY_INTERNAL,
        MDE_REC_BOTH,
        MDE_REC_SKIP
    };

    enum MODULE_FLOW_PERF_PARAM{
        MFP_ON,
        MFP_OFF
    };

    extern const MODULE_VCD_REC_POL PARAM_VCD_REC_POL;
    extern const MODULE_FLOW_PERF_PARAM PARAM_PERF_REC_POL;

}

#endif //KATHRYN_SIMPARAM_H
