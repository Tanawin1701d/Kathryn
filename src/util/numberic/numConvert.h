//
// Created by tanawin on 19/1/2567.
//

#ifndef KATHRYN_NUMCONVERT_H
#define KATHRYN_NUMCONVERT_H

#include <string>
#include "sim/logicRep/valRep.h"

namespace kathryn{


    /**
     * This function is used to convert string like "b010101"
     * or "5b10010"
     * or "20h1ABCF"
     * or "2d"
     * */
    ValRep cvStrToValRep(std::string rawStr);

}

#endif //KATHRYN_NUMCONVERT_H
