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
        //////// sized Verilog literal (<width>'h...), correct for any width incl. >64-bit
        int width = _master->getOperableSlice().getSize();
        return cvtWordsToVerilogLiteral(_master->getConstOprWide(), width);
    }


    std::string ValueGen::decIo(){return "";}



    std::string ValueGen::decVariable(){
        Slice sl = _master->getOperableSlice();

        return "wire [" + std::to_string(sl.stop-1) +
            ": 0]" + getOpr() + " = " + getValStr() + ";";
    }

    std::string ValueGen::decOp(){return "";}

}