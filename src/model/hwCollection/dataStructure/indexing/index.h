//
// Created by tanawin on 20/9/25.
//

#ifndef KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_INDEXING_INDEX_H
#define KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_INDEXING_INDEX_H
#include "model/hwComponent/abstract/operable.h"

namespace kathryn{


    struct OH{
        Operable& _idx;

        OH(Operable& idx): _idx(idx){}

        Operable& getIdx(){return _idx;}
    };

}

#endif //KATHRYN_SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_INDEXING_INDEX_H
