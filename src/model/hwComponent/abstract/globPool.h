//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBPOOL_H
#define GLOBPOOL_H
#include "operable.h"

namespace kathryn{


    void addToGlobPool(Operable* src);
    std::vector<Operable*>& getGlobPool();
    void cleanGlobPool();

}

#endif //GLOBPOOL_H
