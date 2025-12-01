//
// Created by tanawin on 20/6/2024.
//

#include "wireAutoGen.h"
#include "model/hwComponent/wire/wireAuto.h"

namespace kathryn{

    WireAutoGen::WireAutoGen(
        ModuleGen*          mdGenMaster,
        WireAuto*           wireMaster,
        WIRE_AUTO_GEN_TYPE  autoWireGenType
    ):
    AssignGenBase(mdGenMaster,(Assignable*) wireMaster, (Identifiable*) wireMaster),
     _autoWireGenType(autoWireGenType),
    _master(wireMaster){}

    std::string WireAutoGen::getOpr(){
        if ((_autoWireGenType == WIRE_AUTO_GEN_GLOB_INPUT) ||
            (_autoWireGenType == WIRE_AUTO_GEN_GLOB_OUTPUT)){
            return _master->getVarName();
        }
        return LogicGenBase::getOpr();
    }


    std::string WireAutoGen::decIo(){
        assert(_autoWireGenType != WIRE_AUTO_GEN_INTER);
        Slice sl = _master->getOperableSlice();
        return std::string((_autoWireGenType == WIRE_AUTO_GEN_INPUT || _autoWireGenType == WIRE_AUTO_GEN_GLOB_INPUT) ? "input" : "output") +
        " wire[" + std::to_string(sl.stop-1) +": 0] " + getOpr();
    }

    std::string WireAutoGen::decVariable(){
        Slice sl = _master->getOperableSlice();
        return "wire [" + std::to_string(sl.stop-1) +": 0] " +getOpr() + ";";
    }

    std::string WireAutoGen::decOp(){
        assert(!translatedUpdatePool.isEmpty());
        UpdateEventBase* singleUpdateEvent = translatedUpdatePool.getUpdateEventRef()[0];
        assert(singleUpdateEvent != nullptr);
        assert(singleUpdateEvent->getType() == UET_BASIC);

        CbBaseVerilog cb;
        UEBaseGenEngine* ueGenEngine = singleUpdateEvent->createGenEngine();
        ueGenEngine->genBasicConnect(cb, this);
        delete ueGenEngine;

        std::string result = cb.toString(4);

        return result;
    }


}