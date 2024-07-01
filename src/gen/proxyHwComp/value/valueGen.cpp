//
// Created by tanawin on 20/6/2024.
//

#include "valueGen.h"
#include "model/hwComponent/value/value.h"

namespace kathryn{

    ValueGen::ValueGen(ModuleGen*    mdGenMaster,
                       Val*          master):
    LogicGenBase(mdGenMaster,
                 (Assignable*) master,
                 (Identifiable*) master),

    _master(master){ assert(master != nullptr);}

    std::string ValueGen::decIo(){return "";}

    std::string ValueGen::decVariable(){
        Slice sl = _master->getOperableSlice();

        return "wire [" + std::to_string(sl.stop-1) +
            ": 0]" + getOpr() + " = " + std::to_string(_master->_rawValue) + ";";
    }

    std::string ValueGen::decOp(){return "";}

    bool ValueGen::compare(LogicGenBase* lgb){
        assert(lgb->getLogicCef().comptype == TYPE_VAL);
        auto* rhs = dynamic_cast<ValueGen*>(lgb);
        return checkCerfEqLocally(rhs->_cerf) &&
            (_master->getConstOpr() == rhs->_master->getConstOpr());
    }

}