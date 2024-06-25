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
     _isWireIo(false),
    _master(wireMaster){}

    WireGen::WireGen(
        ModuleGen*    mdGenMaster,
        logicLocalCef cerf,
        Wire*         wireMaster,
        WIRE_IO_TYPE  ioType
        ):
    AssignGenBase(mdGenMaster,std::move(cerf),
        (Assignable*) wireMaster, (Identifiable*) wireMaster),
     _isWireIo(true),
     _ioType(ioType),
    _master(wireMaster){}

    std::string WireGen::decIo(){
        assert(_isWireIo);
        assert((_ioType == WIRE_IO_INPUT) || (_ioType == WIRE_IO_OUTPUT));
        Slice sl = _master->getOperableSlice();
        return std::string((_ioType == WIRE_IO_INPUT) ? "input" : "output") +
            "wire[" + std::to_string(sl.stop-1) +": 0] " + getOpr();
    }

    std::string WireGen::decVariable(){
        assert(!_isWireIo);
        Slice sl = _master->getOperableSlice();
        //////////// it act as wire
        return "reg [" + std::to_string(sl.stop-1) +
                ": 0] " +getOpr() + ";";
    }


    std::string WireGen::decOp(){
        if (_isWireIo){
            if(_ioType == WIRE_IO_INPUT){
                return ""; /////// input no need to do operation
            }
            if (_ioType == WIRE_IO_OUTPUT){
                UpdateEvent* singleUpdateEvent = _master->getUpdateMeta()[0];
                assert(singleUpdateEvent != nullptr);
                return "assign " + getOpr() + " = " +
                    getOprStrFromOpr(singleUpdateEvent->srcUpdateValue) + ";";
            }
        }
        return assignOpBase(false);
    }






}