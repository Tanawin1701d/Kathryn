//
// Created by tanawin on 20/6/2024.
//

#include "wireGen.h"
#include "model/hwComponent/wire/wireIo.h"


#include <utility>


namespace kathryn{

    WireGen::WireGen(
        ModuleGen*    mdGenMaster,
        Wire*         wireMaster
        ):
    AssignGenBase(mdGenMaster, (Assignable*) wireMaster,
        (Identifiable*) wireMaster),
    _ioType(WIRE_IO_NORMAL),
    _master(wireMaster){}

    WireGen::WireGen(
        ModuleGen*    mdGenMaster,
        logicLocalCef cerf,
        Wire*         wireMaster,
        WIRE_IO_TYPE  ioType
        ):
    AssignGenBase(mdGenMaster,(Assignable*) wireMaster, (Identifiable*) wireMaster),
     _ioType(ioType),
    _master(wireMaster){}

    std::string WireGen::getOpr(){
        if ((_ioType == WIRE_IO_INPUT_GLOB) ||
            (_ioType == WIRE_IO_OUTPUT_GLOB)){
            return _master->getVarName();
        }
        return LogicGenBase::getOpr();
    }


    std::string WireGen::decIo(){
        assert((_ioType == WIRE_IO_INPUT) ||
            (_ioType == WIRE_IO_OUTPUT) ||
            (_ioType == WIRE_IO_INPUT_GLOB) ||
            (_ioType == WIRE_IO_OUTPUT_GLOB)
            );
        Slice sl = _master->getOperableSlice();
        return std::string(((_ioType == WIRE_IO_INPUT) || (_ioType == WIRE_IO_INPUT_GLOB)) ? "input" : "output") +
            " wire[" + std::to_string(sl.stop-1) +": 0] " + getOpr();
    }

    std::string WireGen::decVariable(){

        Slice sl = _master->getOperableSlice();
        std::string prefix;

        if ( (_ioType == WIRE_IO_INPUT) || (_ioType == WIRE_IO_INPUT_GLOB)){
            prefix = "wire ";
        }else if (_ioType == WIRE_IO_OUTPUT || (_ioType == WIRE_IO_OUTPUT_GLOB)){
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
        if ((_ioType == WIRE_IO_INPUT) || (_ioType == WIRE_IO_INPUT_GLOB)   ||
            (_ioType == WIRE_IO_OUTPUT)|| (_ioType == WIRE_IO_OUTPUT_GLOB)  ||
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

    bool WireGen::compare(LogicGenBase* lgb){
        assert(lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_WIRE);
        auto* rhs = dynamic_cast<WireGen*>(lgb);
        if( (_ioType == WIRE_IO_OUTPUT) ||
            (_ioType == WIRE_IO_OUTPUT_GLOB) ||
            (_ioType == WIRE_IO_INTER) ||
            (_ioType == WIRE_IO_NORMAL)){
            /////////////// when compare output dep is submodule
            return checkCerfEqLocally(rhs->_cerf) && cmpAssignGenBase(rhs, SUBMOD);
        }
        ///////////// the dep of input wire is master module
        return checkCerfEqLocally(rhs->_cerf) && cmpAssignGenBase(rhs, MASTERMOD);

        ////////////// input dep is master

    }

    GLOB_IO_TYPE WireGen::getGlobIoStatus(){
        _master->checkIntegrity();
        return _master->getGlobIoType();
    }







}