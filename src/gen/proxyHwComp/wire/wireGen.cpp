//
// Created by tanawin on 20/6/2024.
//

#include "wireGen.h"

#include <utility>


namespace kathryn{

    WireGen::WireGen(
        ModuleGen*    mdGenMaster,
        logicLocalCef cerf,
        Wire*         wireMaster
        ):
    AssignGenBase(mdGenMaster,std::move(cerf),
        (Assignable*) wireMaster, (Identifiable*) wireMaster),
    _master(wireMaster){}

    std::string WireGen::decIo(){
        assert(isIoWire);
        Slice sl = _master->getOperableSlice();
        return std::string(isInput ? "input" : "output") +
            "wire[" + std::to_string(sl.stop-1) +": 0] " + getOpr();
    }

    std::string WireGen::decVariable(){
        assert(!isIoWire);
        Slice sl = _master->getOperableSlice();
        //////////// it act as wire
        return "reg [" + std::to_string(sl.stop-1) +
                ": 0] " +getOpr() + ";";
    }


    std::string WireGen::decOp(){
        if (isIoWire){
            if(isInput){
                return ""; /////// input no need to do operation
            }else{
                UpdateEvent* singleUpdateEvent = _master->getUpdateMeta()[0];
                assert(singleUpdateEvent != nullptr);
                return "assign " + getOpr() + " = " +
                    getOprStrFromOpr(singleUpdateEvent->srcUpdateValue) + ";";
            }
        }
        return assignOpBase(false);
    }






}