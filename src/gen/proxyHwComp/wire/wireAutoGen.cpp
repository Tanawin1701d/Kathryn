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
        assert(!translatedUpdateEvent.empty());
        UpdateEvent* singleUpdateEvent = translatedUpdateEvent[0];
        assert(singleUpdateEvent != nullptr);
        return "assign " + getOpr() + " = " +
            getOprStrFromOpr(singleUpdateEvent->srcUpdateValue) + ";";
        ///// return assignOpBase(false);
    }

    bool WireAutoGen::compare(LogicGenBase* lgb){
        assert(lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_WIRE);
        auto* rhs = dynamic_cast<WireAutoGen*>(lgb);
        if( (_autoWireGenType == WIRE_AUTO_GEN_OUTPUT) ||
            (_autoWireGenType == WIRE_AUTO_GEN_GLOB_OUTPUT) ||
            (_autoWireGenType == WIRE_AUTO_GEN_INTER)){
            /////////////// when compare output dep is submodule
            return checkCerfEqLocally(*rhs) && cmpAssignGenBase(rhs, SUBMOD);
        }
        ///////////// the dep of input wire is master module
        return checkCerfEqLocally(*rhs) && cmpAssignGenBase(rhs, MASTERMOD);
    }


}