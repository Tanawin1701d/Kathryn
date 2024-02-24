//
// Created by tanawin on 18/1/2567.
//


#include "assignable.h"
#include "makeComponent.h"
#include "model/hwComponent/value/value.h"

namespace kathryn{

    Operable& getMatchAssignOperable(ull value, const int size){
        makeVal(assUserAutoVal, size, value);
        return assUserAutoVal;
    }

}