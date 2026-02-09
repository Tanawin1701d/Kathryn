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


    std::string ValueGen::getValStr() const{

        if (_master->_rawValue == UINT64_MAX){
            return "-1";
        }

        return std::to_string(_master->_rawValue);
    }


    std::string ValueGen::decIo(){return "";}



    std::string ValueGen::decVariable(){
        Slice sl = _master->getOperableSlice();

        return "wire [" + std::to_string(sl.stop-1) +
            ": 0]" + getOpr() + " = " + getValStr() + ";";
    }

    std::string ValueGen::decOp(){return "";}

}