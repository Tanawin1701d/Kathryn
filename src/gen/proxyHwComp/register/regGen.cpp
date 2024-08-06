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
        return AssignGenBase::assignOpBase(true);
    }

    bool RegGen::compare(LogicGenBase* lgb){
        assert( (lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_REG) ||
                (lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_STATE_REG)  ||
                (lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_COND_WAIT_STATE_REG) ||
                (lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_CYCLE_WAIT_STATE_REG)
        );
        auto* rhs = dynamic_cast<RegGen*>(lgb);
        return checkCerfEqLocally(*rhs) && cmpAssignGenBase(rhs, SUBMOD);
    }

    WIRE_MARKER_TYPE RegGen::getGlobIoStatus(){
        _master->checkIntegrity();
        return _master->getMarker();
    }



}