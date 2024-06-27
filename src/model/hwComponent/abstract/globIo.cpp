//
// Created by tanawin on 26/6/2024.
//

#include "globIo.h"
#include "globPool.h"

namespace kathryn{

void GlobIo::asInputGlob(){
    _globIoType = GLOB_IO_INPUT;
    addToGlobPool(this);
}

void GlobIo::asOutputGlob(){
    _globIoType = GLOB_IO_OUTPUT;
    addToGlobPool(this);
}



}