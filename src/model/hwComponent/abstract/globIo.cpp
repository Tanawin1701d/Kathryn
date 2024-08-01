//
// Created by tanawin on 26/6/2024.
//

#include "globIo.h"

#include <utility>
#include "globPool.h"

namespace kathryn{

void GlobIoItf::asInputGlob(std::string value){
    _globIoType = GLOB_IO_INPUT;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void GlobIoItf::asOutputGlob(std::string value){
    _globIoType = GLOB_IO_OUTPUT;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void GlobIoItf::setIoName(const std::string& value){
    if (value.empty()){
        Identifiable* origin = getOprFromGlobIo()->castToIdent();
        assert(origin != nullptr);
        ioValue = origin->getGlobalName() + "_" + origin->getVarMeta().varName;
    }else{
        ioValue = value;
    }
}

std::string GlobIoItf::getGlobIoName(){
    return ioValue;
}



}