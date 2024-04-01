//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_STORAGEMGM_H
#define KATHRYN_STORAGEMGM_H

#include"kathryn.h"

namespace kathryn{

    class StorageMgmt{
    public:
        const int REGSIZE = 32;
        const int AMTREG  = 32;
        const int IDX_SIZE=  5;
        MemBlock& _myMem;

        /** read section*/
        int amountReadIdx = 0;
        std::vector<Operable*> readIdxs;
        std::vector<Wire*>     readEns;
        std::vector<Wire*>     readFinishes;

        Wire& readIdxMaster;
        Operable& readOutput;

        explicit StorageMgmt(int idxSize, int regSize): REGSIZE(regSize),
                                                        AMTREG(1 << idxSize),
                                                        IDX_SIZE(idxSize),
                                                        _myMem(_make<MemBlock>("_myMem", AMTREG, REGSIZE)),
                                                        readIdxMaster(_make<Wire>("_readIndexer", IDX_SIZE)),
                                                        readOutput(_myMem[readIdxMaster]){}

        void reqReadReg(Reg& x, Operable& idx){

            assert(idx.getOperableSlice().getSize() == IDX_SIZE);
            assert(x.getOperableSlice().getSize() == REGSIZE);

            readIdxs.push_back(&idx);
            readEns   .push_back(&_make<Wire>("startReadReg_" + std::to_string(amountReadIdx), 1));
            readFinishes.push_back(&_make<Wire>("notifyReadReg_" + std::to_string(amountReadIdx), 1));
            amountReadIdx++;

            cwhile(true){
                par {
                    *readEns[readFinishes.size() - 1] = 1;
                    cif (*readFinishes[readFinishes.size() - 1]) {
                        x <<= readOutput;
                        sbreak
                    }
                    celse {
                        x <<= 0;
                    }
                }
            }
        }


        void reqWriteReg(Reg& x, Operable& idx){
            assert(idx.getOperableSlice().getSize() == IDX_SIZE);
            assert(x.getOperableSlice().getSize() == REGSIZE);
            _myMem[idx] <<= x;
        }

        void buildReadFlow(){

            cwhile(true){

                zif(*readEns[0]) {
                    readIdxMaster = *readIdxs[0];
                    (*readFinishes[0]) = 1;
                }
                for (int i = 1;  i < readEns.size(); i++){
                    zelif(*readEns[i]){
                        readIdxMaster = *readIdxs[i];
                        (*readFinishes[i]) = 1;
                    }
                }

            }

        }

    };
}

#endif //KATHRYN_STORAGEMGM_H
