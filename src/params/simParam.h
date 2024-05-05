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

    extern MODULE_VCD_REC_POL PARAM_VCD_REC_POL;

}

#endif //KATHRYN_SIMPARAM_H
