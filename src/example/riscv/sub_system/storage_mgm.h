//
// Created by tanawin on 1/4/2567.
//

#ifndef KATHRYN_STORAGEMGM_H
#define KATHRYN_STORAGEMGM_H

#include "kathryn.h"

namespace kathryn{


    namespace riscv {

        class StorageMgmt {
        public:
            const int ROW_WIDTH;
            const int AMT_ROW  ;
            const int IDX_SIZE ;
            MemBlock& _myMem;

            /** read section*/
            int amount_read_idx = 0;
            std::vector<Operable*> read_address;
            std::vector<Operable*> read_ens;
            std::vector<Wire*>     read_finishes;

            Wire     &read_idx_master;
            MemBlockEleHolder&read_output;

            explicit StorageMgmt(int idx_size, int row_width) : ROW_WIDTH    (row_width),
                                                              AMT_ROW      (1 << idx_size),
                                                              IDX_SIZE     (idx_size),
                                                              _myMem       (make_opr_mem("_myMem",AMT_ROW, ROW_WIDTH)),
                                                              read_idx_master(make_opr_wire("_readIndexer",IDX_SIZE)),
                                                              read_output   (_myMem[read_idx_master]){}

            Operable& add_reader(Operable& read_en, Operable &address) {

                assert(address.get_operable_slice().get_size() == IDX_SIZE);

                read_address .push_back(&address);
                read_ens     .push_back(&read_en);
                read_finishes.push_back(&make_opr_wire("notifyReadReg_" + std::to_string(amount_read_idx),1));
                amount_read_idx++;
                return *(*read_finishes.rbegin());
            }


            void req_write_req(Operable& en, Operable& idx, Operable& data) {
                assert(idx.get_operable_slice().get_size()  == IDX_SIZE);
                assert(data.get_operable_slice().get_size() == ROW_WIDTH);
                assert(en.get_operable_slice().get_size() == 1);
                zif (en){
                    _myMem[idx] <<= data;
                }
            }

            void build_read_flow() {

                assert(!read_ens.empty());

                cwhile(true) {

                    zif(*read_ens[read_ens.size()-1]) {
                        read_idx_master = *read_address[read_ens.size()-1];
                        (*read_finishes[read_ens.size()-1]) = 1;
                    }
                    for (int i = ((int)read_ens.size())-2; i >= 0; i--) {
                        zelif(*read_ens[i]) {
                            read_idx_master = *read_address[i];
                            (*read_finishes[i]) = 1;
                        }
                    }

                }

            }

        };

    }
}

#endif //KATHRYN_STORAGEMGM_H
