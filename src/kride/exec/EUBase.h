//
// Created by tanawin on 8/4/2025.
//

#ifndef KATHRYN_SRC_KRIDE_EXEC_EUBASE_H
#define KATHRYN_SRC_KRIDE_EXEC_EUBASE_H

#include "kride/incl.h"
#include "kride/reg/rrf.h"


namespace kathryn{

    struct EUBase: Module{

        D_ALL& d;
        D_IO_RSV& dc;

        EUBase(D_ALL& din, D_IO_RSV& dcIn):
        d(din), dc(dcIn){}

        void writeReg(Slot& sl, Operable& resOpr){
            zif(sl.at("regWr"))
                    d.rrf->writeData(resOpr, sl.at("rrfTag"));
        }



    };

}

#endif //KATHRYN_SRC_KRIDE_EXEC_EUBASE_H
