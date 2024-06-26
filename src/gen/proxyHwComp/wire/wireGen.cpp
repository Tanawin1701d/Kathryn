//
// Created by tanawin on 20/6/2024.
//

#include "wireGen.h"
#include "model/hwComponent/wire/wireIo.h"


#include <utility>


namespace kathryn{

    WireGen::WireGen(
        ModuleGen*    mdGenMaster,
        logicLocalCef cerf,
        Wire*         wireMaster
        ):
    AssignGenBase(mdGenMaster,std::move(cerf),
        (Assignable*) wireMaster, (Identifiable*) wireMaster),
    _ioType(WIRE_IO_NORMAL),
    _master(wireMaster){}

    WireGen::WireGen(
        ModuleGen*    mdGenMaster,
        logicLocalCef cerf,
        Wire*         wireMaster,
        WIRE_IO_TYPE  ioType
        ):
    AssignGenBase(mdGenMaster,std::move(cerf),
        (Assignable*) wireMaster, (Identifiable*) wireMaster),
     _ioType(ioType),
    _master(wireMaster){}

    std::string WireGen::decIo(){
        assert((_ioType == WIRE_IO_INPUT) || (_ioType == WIRE_IO_OUTPUT));
        Slice sl = _master->getOperableSlice();
        return std::string((_ioType == WIRE_IO_INPUT) ? "input" : "output") +
            " wire[" + std::to_string(sl.stop-1) +": 0] " + getOpr();
    }

    std::string WireGen::decVariable(){

        Slice sl = _master->getOperableSlice();
        std::string prefix;

        if (_ioType == WIRE_IO_INPUT){
            prefix = "wire ";
        }else if (_ioType == WIRE_IO_OUTPUT){
            prefix = "wire ";
        }else if (_ioType == WIRE_IO_INTER){
            prefix = "wire ";
        }else if (_ioType == WIRE_IO_NORMAL){
            prefix = "reg ";
        }

        return prefix + " [" + std::to_string(sl.stop-1) +
                ": 0] " +getOpr() + ";";
    }


    std::string WireGen::decOp(){
        if ((_ioType == WIRE_IO_INPUT) ||
            (_ioType == WIRE_IO_OUTPUT)||
            (_ioType == WIRE_IO_INTER)
           ){
            assert(!translatedUpdateEvent.empty());
            UpdateEvent* singleUpdateEvent = translatedUpdateEvent[0];
            assert(singleUpdateEvent != nullptr);
            return "assign " + getOpr() + " = " +
                getOprStrFromOpr(singleUpdateEvent->srcUpdateValue) + ";";
        }

        return assignOpBase(false);
    }






}