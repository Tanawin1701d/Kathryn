//
// Created by tanawin on 9/12/25.
//

#ifndef EXAMPLE_O3_CORE_STOREBUF_H
#define EXAMPLE_O3_CORE_STOREBUF_H

#include "search_idx.h"
#include "slotParam.h"

namespace kathryn::o3{

    struct StoreBuf{   ///MD STOREBUF
        ///// main data
        LdStStage& lss;                                 ///CTRL_HC+DATA_HC STOREBUF
        BroadCast& bc;                                  ///CTRL_HC STOREBUF
        Table _table{smStoreBuf, STBUF_ENT_NUM};        ///CTRL_HWD STOREBUF
        mMem(daytas , STBUF_ENT_NUM, DATA_LEN);         ///DATA_HWD STOREBUF
        ///// meta data
        mReg(finPtr, STBUF_ENT_SEL); ////               ///CTRL_HWD STOREBUF
        mReg(comPtr, STBUF_ENT_SEL); //// completePtr   ///CTRL_HWD STOREBUF
        mReg(retPtr, STBUF_ENT_SEL); //// retirePtr     ///CTRL_HWD STOREBUF
        ///// search result of zero bit
        SearchResult nb1, ne1, nb0;   ///CTRL_HWD STOREBUF
        mWire(fullNext, 1);  // full after kill mispredict    ///CTRL_HWD STOREBUF
        mWire(emptyNext, 1); // empty after kill mispredict   ///CTRL_HWD STOREBUF



        StoreBuf(LdStStage& lss, BroadCast& bc):       ///CTRL_HC+DATA_HC STOREBUF
        lss(lss),                                      ///CTRL_HC+DATA_HC STOREBUF
        bc(bc),                                        ///CTRL_HC STOREBUF
        nb1(searchIdx(_table, 1, true , bc, true )),   ///CTRL_HC STOREBUF
        ne1(searchIdx(_table, 1, false, bc, true )),   ///CTRL_HC STOREBUF
        nb0(searchIdx(_table, 0, true , bc, true )){   ///CTRL_HC STOREBUF
            finPtr.makeResetEvent();                   ///CTRL_DT STOREBUF
            comPtr.makeResetEvent();                   ///CTRL_DT STOREBUF
            retPtr.makeResetEvent();                   ///CTRL_DT STOREBUF
            _table.makeColResetEvent(busy);            ///CTRL_DT STOREBUF
            _table.makeColResetEvent(complete);        ///CTRL_DT STOREBUF
            _table.makeColResetEvent(spec);            ///CTRL_DT STOREBUF

        }

        void onCommit(){                      ///HLH STOREBUF
            comPtr <<= comPtr + 1;            ///CTRL_CL STOREBUF
            _table[comPtr](complete) <<= 1;   ///CTRL_DT STOREBUF
        }

        void onNewEntry(RegSlot& src, opr& dayta, opr& memAddr){   ///DATA_HC STOREBUF

            //// it may be overrided with onMisPred
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);               ///CTRL_CL STOREBUF
            _table[finPtr](complete) <<= 0;                             ///CTRL_DT STOREBUF
            _table[finPtr](mem_addr) <<= memAddr;                       ///DATA_DT STOREBUF
            _table[finPtr]           <<= src; /// busy, spec, specTag   ///DATA_DT STOREBUF
            daytas[finPtr]           <<= dayta;                         ///DATA_DT STOREBUF
            finPtr                   <<= (finPtr + 1);                  ///CTRL_CL STOREBUF
            zif(bc.checkIsSuc(src)){                                    ///CTRL_CL STOREBUF
                _table[finPtr](spec) <<= 0;                             ///CTRL_DT STOREBUF
            }
            SET_ASM_PRI_TO_AUTO();                                      ///CTRL_CL STOREBUF
        }

        void onMisPred(opr& fixTag){                           ///CTRL_HC STOREBUF
            ////// destroy the internal buffer
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){   ///HLH STOREBUF
                //////// do bypass the system
                zif ( lhs(spec) &                              ///CTRL_CL STOREBUF
                    ((lhs(specTag)&fixTag) != 0)){             ///CTRL_CL STOREBUF
                    lhs(busy) <<= 0;                           ///CTRL_DT STOREBUF
                }
            });
            ////// refit meta data
            /// if empty or full the value of fin is equal to old system
            zif (~emptyNext && ~fullNext){   ///CTRL_CL STOREBUF
                finPtr <<= ne1.sIdx + 1; //// pre declare that there is no bubble   ///CTRL_CL STOREBUF
                ///// there at least 1 and 1 begin detection and 1 end detection must have it
                zif( (nb1.sIdx == 0) && (ne1.sIdx == (_table.getNumRow()-1)) ){   ///CTRL_CL STOREBUF
                    finPtr <<= nb0.sIdx;   ///CTRL_DT STOREBUF
                }
            }

            zif(emptyNext){ ///// the system is empty next     ///CTRL_CL STOREBUF
                comPtr <<= finPtr;                             ///CTRL_DT STOREBUF
                retPtr <<= finPtr;                             ///CTRL_DT STOREBUF
            }
        }

        void onSucPred(opr& sucTag){                           ///CTRL_HC STOREBUF
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){   ///HLH STOREBUF
                //////// do bypass the system
                zif (lhs(spec) &                               ///CTRL_CL STOREBUF
                    (lhs(specTag) == sucTag)){                 ///CTRL_CL STOREBUF
                    lhs(spec) <<= 0;                           ///CTRL_DT STOREBUF
                }
            });
        }

        opr& isFull(){return (finPtr == retPtr) && (_table[finPtr](busy).v());}   ///CTRL_CL STOREBUF



        std::pair<opr&, opr&> searchNewest(opr& addr){                            ///DATA_HC STOREBUF

            /// find newst system.
             auto[result, binIdx] = _table.findMBO_BIDX(true, finPtr,             ///HLH STOREBUF
                 [&](RegSlot& lhs)->opr&{                                         ///HLH STOREBUF
                 return lhs(busy) & (lhs(mem_addr) == addr);                      ///CTRL_CL STOREBUF
             });
            return {result(busy) & (result(mem_addr) == addr), daytas[binIdx]};   ///CTRL_HC+DATA_HC STOREBUF
        }

        void flow(){ //// retire and do other thing   ///HLH STOREBUF
            fullNext  = ~(nb0.sValid);                ///CTRL_CL STOREBUF
            emptyNext = ~(ne1.sValid);                ///CTRL_CL STOREBUF

            ///// retire system
            WireSlot retireSlot(_table[retPtr].v(), "retireSlot");   ///CTRL_DT+DATA_DT STOREBUF
            lss.dmem_rwaddr = retireSlot(mem_addr);                  ///DATA_DT STOREBUF
            lss.dmem_wdata = daytas[retPtr];                         ///DATA_DT STOREBUF

            zif (retireSlot(busy) & retireSlot(complete) & (~bc.isBrMissPred())){                                                   ///CTRL_CL STOREBUF
                zif(lss.dmem_we = 1){ ///// try to make it equal to one if not update will not occur because memory hold them all   ///CTRL_CL STOREBUF
                    retPtr <<= (retPtr + 1);                                                                                        ///CTRL_CL STOREBUF
                    _table[retPtr](busy)     <<= 0;                                                                                 ///CTRL_DT STOREBUF
                    _table[retPtr](complete) <<= 0;                                                                                 ///CTRL_DT STOREBUF
                }
            }
        }
    };

}

#endif //EXAMPLE_O3_CORE_STOREBUF_H
