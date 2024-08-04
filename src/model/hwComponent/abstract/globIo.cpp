//
// Created by tanawin on 26/6/2024.
//

#include "globIo.h"

#include <utility>
#include "globPool.h"

namespace kathryn{

void GlobIo::asInputGlob(std::string value){
    _ioType = GLOB_IO_INPUT;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void GlobIo::asInput(std::string value){
    _ioType = GLOB_IO_INPUT_MD;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void GlobIo::asOutputGlob(std::string value){
    _ioType = GLOB_IO_OUTPUT;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void GlobIo::asOutput(std::string value){
    _ioType = GLOB_IO_OUTPUT_MD;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void GlobIo::setIoName(const std::string& value){
    if (value.empty()){
        Identifiable* origin = getOprFromGlobIo()->castToIdent();
        assert(origin != nullptr);
        ioValue = origin->getGlobalName() + "_" + origin->getVarMeta().varName;
    }else{
        ioValue = value;
    }
}

std::string GlobIo::getGlobIoName(){
    return ioValue;
}



}