//
// Created by tanawin on 2/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_DECODER_DECODER_H
#define KATHRYN_SRC_KRIDE_DECODER_DECODER_H

#include "kride/incl.h"

namespace kathryn{

    struct Decoder: Module{

        D_INSTR  instrs;
        D_DECODE decs[DEC_AMT];

        Decoder(D_INSTR inss): instrs(inss){}

        void flow() override{

            pip(DEC){
                pipTran(DP){
                    pipSync(SPEC_GEN);
                    decs[0].decode(instrs.instr0);
                    decs[1].decode(instrs.instr1);
                }
            }
        }
    };

}

#endif //KATHRYN_SRC_KRIDE_DECODER_DECODER_H
