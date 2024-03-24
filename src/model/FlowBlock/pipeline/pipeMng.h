//
// Created by tanawin on 24/3/2567.
//

#ifndef KATHRYN_PIPEMNG_H
#define KATHRYN_PIPEMNG_H

#include "model/hwComponent/expression/expression.h"

namespace kathryn{

    class PipeMeta{
    private:
        expression* availSendSignal;
        expression* notifyToSendSignal;
    };



}

#endif //KATHRYN_PIPEMNG_H
