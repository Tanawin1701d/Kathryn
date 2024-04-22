//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_STORAGEMGM_H
#define KATHRYN_STORAGEMGM_H

#include"kathryn.h"

namespace kathryn{


    namespace riscv {

        class StorageMgmt {
        public:
            const int REGSIZE  = 32;
            const int AMTREG   = 32;
            const int IDX_SIZE = 5;
            MemBlock &_myMem;

            /** read section*/
            int amountReadIdx = 0;
            std::vector<Operable*> readAddress;
            std::vector<Operable*> readEns;
            std::vector<Wire*>     readFinishes;

            Wire     &readIdxMaster;
            MemBlockEleHolder&readOutput;

            explicit StorageMgmt(int idxSize, int regSize) : REGSIZE(regSize),
                                                             AMTREG(1 << idxSize),
                                                             IDX_SIZE(idxSize),
                                                             _myMem(_make<MemBlock>("_myMem", AMTREG, REGSIZE)),
                                                             readIdxMaster(_make<Wire>("_readIndexer", IDX_SIZE)),
                                                             readOutput(_myMem[readIdxMaster]){}

            Operable& addReader(Operable& readEn, Operable &address) {

                assert(address.getOperableSlice().getSize() == IDX_SIZE);


                readAddress    .push_back(&address);
                readEns     .push_back(&readEn);
                readFinishes.push_back(&_make<Wire>("notifyReadReg_" + std::to_string(amountReadIdx), 1));
                amountReadIdx++;
                return *(*readFinishes.rbegin());
            }


            void reqWriteReq(Operable& x, Operable &idx) {
                assert(idx.getOperableSlice().getSize() == IDX_SIZE);
                assert(x.getOperableSlice().getSize() == REGSIZE);
                _myMem[idx] <<= x;
            }

            void buildReadFlow() {

                assert(!readEns.empty());

                cwhile(true) {

                    zif(*readEns[readEns.size()-1]) {
                        readIdxMaster = *readAddress[0];
                        (*readFinishes[0]) = 1;
                    }
                    for (int i = ((int)readEns.size())-2; i >= 0; i--) {
                        zelif(*readEns[i]) {
                            readIdxMaster = *readAddress[i];
                            (*readFinishes[i]) = 1;
                        }
                    }

                }

            }

        };

    }
}

#endif //KATHRYN_STORAGEMGM_H
