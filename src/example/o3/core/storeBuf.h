//
// Created by tanawin on 9/12/25.
//

#ifndef EXAMPLE_O3_CORE_STOREBUF_H
#define EXAMPLE_O3_CORE_STOREBUF_H


#include "kathryn.h"
#include "parameter.h"
#include "search_idx.h"
#include "slotParam.h"

namespace kathryn::o3{

    struct StoreBuf{
        ///// main data
        LdStStage& lss;
        BroadCast& bc;
        Table _table{smStoreBuf, STBUF_ENT_NUM};
        mMem(daytas , STBUF_ENT_NUM, DATA_LEN);
        ///// meta data
        mReg(finPtr, STBUF_ENT_SEL); ////
        mReg(comPtr, STBUF_ENT_SEL); //// completePtr
        mReg(retPtr, STBUF_ENT_SEL); //// retirePtr
        ///// search result of zero bit
        SearchResult nb1, ne1, nb0;
        mWire(fullNext, 1);  /// full after kill mispredict
        mWire(emptyNext, 1); /// empty after kill mispredict



        StoreBuf(LdStStage& lss, BroadCast& bc):
        lss(lss),
        bc(bc),
        nb1(searchIdx(_table, 1, true , bc, true )),
        ne1(searchIdx(_table, 1, false, bc, true )),
        nb0(searchIdx(_table, 0, true , bc, true )){
            finPtr.makeResetEvent();
            comPtr.makeResetEvent();
            retPtr.makeResetEvent();
            _table.makeColResetEvent(busy);

        }

        void onCommit(){
            comPtr <<= comPtr + 1;
            _table[comPtr](complete) <<= 1;
        }

        void onNewEntry(RegSlot& src, opr& dayta, opr& memAddr){

            //// it may be overrided with onMisPred
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);
            _table[finPtr](complete) <<= 0;
            _table[finPtr](mem_addr) <<= memAddr;
            _table[finPtr]           <<= src; /// busy, spec, specTag
            daytas[finPtr]           <<= dayta;
            finPtr                   <<= (finPtr + 1);
            zif(bc.checkIsSuc(src)){
                _table[finPtr](spec) <<= 0;
            }
            SET_ASM_PRI_TO_AUTO();
        }

        void onMisPred(opr& fixTag){
            ////// destroy the internal buffer
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                //////// do bypass the system
                zif ( lhs(spec) &
                    ((lhs(specTag)&fixTag) != 0)){
                    lhs(busy) <<= 0;
                }
            });
            ////// refit meta data
            /// if empty or full the value of fin is equal to old system
            zif (~emptyNext && ~fullNext){
                finPtr <<= ne1.sIdx + 1; //// pre declare that there is no bubble
                ///// there at least 1 and 1 begin detection and 1 end detection must have it
                zif( (nb1.sIdx == 0) && (ne1.sIdx == (_table.getNumRow()-1)) ){
                    finPtr <<= nb0.sIdx;
                }
            }

            zif(emptyNext){ ///// the system is empty next
                comPtr <<= finPtr;
                retPtr <<= finPtr;
            }
        }

        void onSucPred(opr& sucTag){
            _table.doCusLogic([&](RegSlot& lhs, int rowIdx){
                //////// do bypass the system
                zif (lhs(spec) &
                    (lhs(specTag) == sucTag)){
                    lhs(spec) <<= 0;
                }
            });
        }

        opr& isFull(){return (finPtr == retPtr) && (_table[finPtr](busy).v());}



        std::pair<opr&, opr&> searchNewest(opr& addr){

            /// find newst system.
             auto[result, binIdx] = _table.findMBO_BIDX(true, finPtr,
                 [&](RegSlot& lhs)->opr&{
                 return lhs(busy) & (lhs(mem_addr) == addr);
             });
            return {result(busy) & (result(mem_addr) == addr), daytas[binIdx]};
        }

        void flow(){ //// retire and do other thing
            fullNext  = ~(nb0.sValid);
            emptyNext = ~(ne1.sValid);

            ///// retire system
            WireSlot retireSlot(_table[retPtr].v(), "retireSlot");
            lss.dmem_rwaddr = retireSlot(mem_addr);
            lss.dmem_wdata = daytas[retPtr];

            zif (retireSlot(busy) & retireSlot(complete) & (~bc.isBrMissPred())){
                zif(lss.dmem_we = 1){ ///// try to make it equal to one if not update will not occur because memory hold them all
                    retPtr <<= (retPtr + 1);
                    _table[retPtr](busy)     <<= 0;
                    _table[retPtr](complete) <<= 0;
                }
            }



        }



    };

}

#endif //EXAMPLE_O3_CORE_STOREBUF_H