//
// Created by tanawin on 5/4/2567.
//

#ifndef KATHRYN_ELEMENT_H
#define KATHRYN_ELEMENT_H


#include "kathryn.h"
#include "parameter.h"
#include "lib/instr/instr_base.h"


namespace kathryn{

    namespace riscv{

        struct FETCH_DATA{
            SyncMeta sync{"fetch_sync"};
            m_reg(fetch_pc    , MEM_ADDR_IDX);
            m_reg(fetch_nextpc, MEM_ADDR_IDX);
            m_reg(fetch_instr, XLEN);
        };

        struct DECODE_DATA{
            InstrRepo repo;
            m_reg(pc, XLEN);
            m_reg(next_pc, XLEN);
            SyncMeta sync{"decode_sync"};

            explicit DECODE_DATA(Operable& instr):
                repo(XLEN, AMT_DEC_SRC_REG,
                AMT_DEC_DES_REG ,XLEN,&instr){}
        };

        struct EXEC_DATA{
            SyncMeta sync{"exec_sync"};
            OPR_HW   wb_data{XLEN, REG_IDX,0, false}; //// write back data
        };

        struct WRITE_BACK_DATA{

            SyncMeta sync{"wb_sync"};
        };

        struct BYPASS_DATA{
            m_wire(idx, REG_IDX);
            m_wire(value, XLEN);
        };


        struct CORE_DATA{
            FETCH_DATA  ft;
            DECODE_DATA dc{ft.fetch_instr};
            EXEC_DATA   ex;
            WRITE_BACK_DATA wb;
            BYPASS_DATA bp;

            m_reg (pc, MEM_ADDR_IDX); //// the fetching pc

            void kill(){
                ft.sync.kill_slave(true);
                dc.sync.kill_slave(true);
            }

            void change_pc(opr& new_pc){
                SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
                pc <<= new_pc;
                SET_ASM_PRI_TO_AUTO();
            }

        };

    }

}

#endif //KATHRYN_ELEMENT_H
