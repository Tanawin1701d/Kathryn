//
// Created by tanawin on 7/4/2567.
//

#ifndef KATHRYN_REGACCESS_H
#define KATHRYN_REGACCESS_H


#include "decode.h"

namespace kathryn{

    namespace riscv{

        class RegAccess{

        public:
            UOp regAccessUop;
            makeReg(x,1);


            void flow(Decode& dec, RegMgmt& regArb){
                UOp& decUop = dec.decInstr;

                pipBlk{
                    par{
                        regAccessUop <<= decUop;
                        seq {
                            cwhile( regArb.isOverlapWithWriteReg(decUop.regData[RS_1]) |
                                    regArb.isOverlapWithWriteReg(decUop.regData[RS_2]) ///// r3 is used only in imm purpose
                                    ) {
                                x <<= x;
                            }
                                    /////// reg access can be parallel but in current version arbiter is only single port
                            regArb.reqReg(decUop.regData[RS_1]);
                            regArb.reqReg(decUop.regData[RS_2]);
                        }
                    }
                }
            }
        };
    }


}

#endif //KATHRYN_REGACCESS_H
