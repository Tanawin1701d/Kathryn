//
// Created by tanawin on 20/6/2024.
//

#include "wireGen.h"
#include "model/hwComponent/wire/wireAutoGen.h"


#include <utility>


namespace kathryn{

    WireGen::WireGen(
        ModuleGen*    mdGenMaster,
        Wire*         wireMaster
        ):
    AssignGenBase(mdGenMaster, (Assignable*) wireMaster,
        (Identifiable*) wireMaster),
    _ioType(WIRE_AUTO_GEN_NORMAL),
    _master(wireMaster){}

    WireGen::WireGen(
        ModuleGen*    mdGenMaster,
        Wire*         wireMaster,
        WIRE_AUTO_GEN_TYPE  ioType
        ):
    AssignGenBase(mdGenMaster,(Assignable*) wireMaster, (Identifiable*) wireMaster),
     _ioType(ioType),
    _master(wireMaster){}

    std::string WireGen::getOpr(){
        if ((_ioType == WIRE_AUTO_GEN_GLOB_INPUT) ||
            (_ioType == WIRE_AUTO_GEN_GLOB_OUTPUT)){
            return _master->getVarName();
        }
        return LogicGenBase::getOpr();
    }


    std::string WireGen::decIo(){
        assert((_ioType == WIRE_AUTO_GEN_INPUT) ||
            (_ioType == WIRE_AUTO_GEN_OUTPUT) ||
            (_ioType == WIRE_AUTO_GEN_GLOB_INPUT) ||
            (_ioType == WIRE_AUTO_GEN_GLOB_OUTPUT)
            );
        Slice sl = _master->getOperableSlice();
        return std::string(((_ioType == WIRE_AUTO_GEN_INPUT) || (_ioType == WIRE_AUTO_GEN_GLOB_INPUT)) ? "input" : "output") +
            " wire[" + std::to_string(sl.stop-1) +": 0] " + getOpr();
    }

    std::string WireGen::decVariable(){

        Slice sl = _master->getOperableSlice();
        std::string prefix;

        if ( (_ioType == WIRE_AUTO_GEN_INPUT) || (_ioType == WIRE_AUTO_GEN_GLOB_INPUT)){
            prefix = "wire ";
        }else if (_ioType == WIRE_AUTO_GEN_OUTPUT || (_ioType == WIRE_AUTO_GEN_GLOB_OUTPUT)){
            prefix = "wire ";
        }else if (_ioType == WIRE_AUTO_GEN_INTER){
            prefix = "wire ";
        }else if (_ioType == WIRE_AUTO_GEN_NORMAL){
            prefix = "reg ";
        }

        return prefix + " [" + std::to_string(sl.stop-1) +
                ": 0] " +getOpr() + ";";
    }


    std::string WireGen::decOp(){
        if ((_ioType == WIRE_AUTO_GEN_INPUT) || (_ioType == WIRE_AUTO_GEN_GLOB_INPUT)   ||
            (_ioType == WIRE_AUTO_GEN_OUTPUT)|| (_ioType == WIRE_AUTO_GEN_GLOB_OUTPUT)  ||
            (_ioType == WIRE_AUTO_GEN_INTER)
           ){
            assert(!translatedUpdateEvent.empty());
            UpdateEvent* singleUpdateEvent = translatedUpdateEvent[0];
            assert(singleUpdateEvent != nullptr);
            return "assign " + getOpr() + " = " +
                getOprStrFromOpr(singleUpdateEvent->srcUpdateValue) + ";";
        }

        return assignOpBase(false);
    }

    bool WireGen::compare(LogicGenBase* lgb){
        assert(lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_WIRE);
        auto* rhs = dynamic_cast<WireGen*>(lgb);
        if( (_ioType == WIRE_AUTO_GEN_OUTPUT) ||
            (_ioType == WIRE_AUTO_GEN_GLOB_OUTPUT) ||
            (_ioType == WIRE_AUTO_GEN_INTER) ||
            (_ioType == WIRE_AUTO_GEN_NORMAL)){
            /////////////// when compare output dep is submodule
            return checkCerfEqLocally(*rhs) && cmpAssignGenBase(rhs, SUBMOD);
        }
        ///////////// the dep of input wire is master module
        return checkCerfEqLocally(*rhs) && cmpAssignGenBase(rhs, MASTERMOD);

        ////////////// input dep is master

    }

    GLOB_IO_TYPE WireGen::getGlobIoStatus(){
        _master->checkIntegrity();
        return _master->getGlobIoType();
    }

}