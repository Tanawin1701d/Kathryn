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
            const int ROW_WIDTH;
            const int AMT_ROW  ;
            const int IDX_SIZE ;
            MemBlock& _myMem;

            /** read section*/
            int amountReadIdx = 0;
            std::vector<Operable*> readAddress;
            std::vector<Operable*> readEns;
            std::vector<Wire*>     readFinishes;

            Wire     &readIdxMaster;
            MemBlockEleHolder&readOutput;

            explicit StorageMgmt(int idxSize, int rowWidth) : ROW_WIDTH    (rowWidth),
                                                              AMT_ROW      (1 << idxSize),
                                                              IDX_SIZE     (idxSize),
                                                              _myMem       (makeOprMem("_myMem",AMT_ROW, ROW_WIDTH)),
                                                              readIdxMaster(makeOprWire("_readIndexer",IDX_SIZE)),
                                                              readOutput   (_myMem[readIdxMaster]){}

            Operable& addReader(Operable& readEn, Operable &address) {

                assert(address.getOperableSlice().getSize() == IDX_SIZE);

                readAddress .push_back(&address);
                readEns     .push_back(&readEn);
                readFinishes.push_back(&makeOprWire("notifyReadReg_" + std::to_string(amountReadIdx),1));
                amountReadIdx++;
                return *(*readFinishes.rbegin());
            }


            void reqWriteReq(Operable& en, Operable& idx, Operable& data) {
                assert(idx.getOperableSlice().getSize()  == IDX_SIZE);
                assert(data.getOperableSlice().getSize() == ROW_WIDTH);
                assert(en.getOperableSlice().getSize() == 1);
                zif (en){
                    _myMem[idx] <<= data;
                }
            }

            void buildReadFlow() {

                assert(!readEns.empty());

                cwhile(true) {

                    zif(*readEns[readEns.size()-1]) {
                        readIdxMaster = *readAddress[readEns.size()-1];
                        (*readFinishes[readEns.size()-1]) = 1;
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
