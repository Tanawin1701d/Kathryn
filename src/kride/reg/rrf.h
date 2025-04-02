//
// Created by tanawin on 25/3/2025.
//

#ifndef src_kride_reg_RRF_H
#define src_kride_reg_RRF_H

#include "kride/incl.h"

namespace kathryn{

    struct Rrf: public Module{
        Table phyRegs;
        Rrf(int x):
            phyRegs("phyRegs",
            {"valid", "data"},
             {1      ,   32},
             6){}

        void flow() override{
            ////// offer from EXEC to set valid and set data
            ////// provide priority
            for (int idx = 0; idx < EXEC_WRITE_NUM; idx++) {
                offer(RRF_exec + std::to_string(idx)){
                    ofc(EXEC + std::to_string(idx)){
                        ////// set data to system
                        ////// TODO set data data
                    }
                }
            }
            ////// offer from allocation
            for (int idx = 0; idx < RENAME_NUM; idx++){
                offer(RRF_alloc + std::to_string(idx)){
                    ofc(ALLOC + std::to_string(idx)){
                        ////// TODO set data data
                    }
                }
            }
        }

        void writeData(Operable& data, Operable& idx){
            phyRegs.assign("data", &data,idx, true);
        }
    };


}

#endif //src/kride/reg_RRF_H
