//
// Created by tanawin on 12/4/2025.
//

#ifndef KATHRNY_SRC_KRIDE_EXEC_LDUNIT_H
#define KATHRNY_SRC_KRIDE_EXEC_LDUNIT_H

#include "kride/krideParam.h"
#include "EUBase.h"

namespace kathryn{

    struct LdstUnit: EUBase{

        mWire(finalLd, DATA_LEN);

        LdstUnit(D_ALL& din, D_IO_RSV& dcIn):
        EUBase(din, dcIn){}

        Operable& comitable(Slot& opSl){
            //// store -> not empty
            return ~opSl.at("regWr") | (~d.stb.full);
        }

        void flow() override{
            Slot& opSl = d.rsvLstd.issueBuf; //// op mean for Operating Load or Store
            Slot& cmSl = d.ldst.cmSlot;

            pip(EX_LDST){
                intrRstAndStart(d.shouldInv(opSl));
                ////////// TODO mem request do operation (we wait for efficient mux)
                pipTranWhen(EX_LDSTC, comitable(opSl)){
                    cmSl <<= opSl; //// for rrfTag and regWrite
                    cmSl <<= d.stb.ldBp; //// for stb by pass Data
                    cmSl <<= d.mem.dataR; //// for mem load data
                }
            }
            /////// multiplex the final load data
            zif (cmSl.at("hit")) {finalLd = cmSl.at("sbpData");}
            zelse{finalLd = cmSl.at("drData");}

            pip(EX_LDSTC){
                intrRstAndStart(d.shouldInv(cmSl));
                writeReg(cmSl, finalLd);
            }

        }

    };


}

#endif //KATHRNY_SRC_KRIDE_EXEC_LDUNIT_H
