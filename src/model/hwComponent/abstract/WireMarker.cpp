//
// Created by tanawin on 26/6/2024.
//

#include "WireMarker.h"

#include <utility>
#include "globPool.h"

namespace kathryn{

void WireMarker::asInputGlob(std::string value){
    _marker = WMT_GLOB_INPUT;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void WireMarker::asInput(std::string value){
    _marker = WMT_INPUT_MD;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void WireMarker::asOutputGlob(std::string value){
    _marker = WMT_GLOB_OUTPUT;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void WireMarker::asOutput(std::string value){
    _marker = WMT_OUTPUT_MD;
    setIoName(std::move(value));
    addToGlobPool(this);
}

void WireMarker::setIoName(const std::string& value){
    if (value.empty()){
        Identifiable* origin = getOprFromGlobIo()->castToIdent();
        assert(origin != nullptr);
        ioValue = origin->getGlobalName() + "_" + origin->getVarMeta().varName;
    }else{
        ioValue = value;
    }
}

std::string WireMarker::getGlobIoName(){
    return ioValue;
}



}