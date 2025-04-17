//
// Created by tanawin on 17/4/2025.
//

#ifndef KATHRYN_SRC_LIB_NUMBERIC_INDEXING_H
#define KATHRYN_SRC_LIB_NUMBERIC_INDEXING_H

#include "kathryn.h"

namespace kathryn{

    ///////////// calculate the distance between two pointer
    /// it return the same size as the input if it overflow
    /// it will return zero

    ////// bh = behind / fr = front    |()|(bh)|()|()|(fr)|
    Operable& cirPtrDis(Operable& bhPtr, Operable& frPtr);

}

#endif //KATHRYN_SRC_LIB_NUMBERIC_INDEXING_H
