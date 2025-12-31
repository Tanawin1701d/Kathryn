//
// Created by tanawin on 20/6/2024.
//

#include "regGen.h"
#include "model/hwComponent/register/register.h"

namespace kathryn{

    RegGen::RegGen(ModuleGen* mdGenMaster,
                   Reg* regMaster):
    AssignGenBase(
        mdGenMaster,
        (Assignable*)regMaster,
        (Identifiable*)regMaster
    ),_master(regMaster){}

    std::string RegGen::decIo(){
        assert(false);
    }

    std::string RegGen::decVariable(){
        Slice sl = _master->getOperableSlice();
        //////////// it act as wire
        return "reg [" + std::to_string(sl.stop-1) +
                ": 0] " +getOpr() + ";";
    }

    std::string RegGen::decOp(){
        return AssignGenBase::assignOpBase();
    }

    WIRE_MARKER_TYPE RegGen::getGlobIoStatus(){
        _master->checkIntegrity();
        return _master->getMarker();
    }



}