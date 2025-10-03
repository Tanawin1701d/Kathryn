//
// Created by tanawin on 25/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_IRSV_H
#define KATHRYN_SRC_EXAMPLE_O3_IRSV_H

#include "rsv.h"
#include "search_idx.h"

#define sValid first
#define sIdx   second
namespace kathryn::o3{



    struct IRsv: RsvBase{

        Reg& allocPtr;
        SearchResult b1 ,  e1,  e0;
        SearchResult nb1, ne1, nb0; /// search to fix alloc pointer

        IRsv(SlotMeta meta, int indexSize, std::string debugName, BroadCast& bc):
            RsvBase(smRsvI + meta, 1 << indexSize),
            allocPtr(mOprReg("allocPtr_" + debugName, 1 << indexSize)),
            b1 (searchIdx(_table, 1, true , bc, false)),
            e1 (searchIdx(_table, 1, false, bc, false)),
            e0 (searchIdx(_table, 1, true , bc, false)),
            nb1(searchIdx(_table, 1, true , bc, true )),
            ne1(searchIdx(_table, 1, false, bc, true )),
            nb0(searchIdx(_table, 1, true , bc, true )){
            allocPtr.makeResetEvent();
        }

        void onMisPred(opr& fixTag) override{

            RsvBase::onMisPred(fixTag);
            zif (nb0.sValid){ /// there is empty space for next update
                ////// case 0  is bubble there is 1 atstart and 1 at the end
                zif((nb1.sIdx == 0) && (ne1.sIdx == (_table.getNumRow()-1))){
                    allocPtr <<= nb0.sIdx;
                }zelse{
                    allocPtr <<= (nb1.sIdx+1);
                }
            }
            /// incase update when entrance the filler at rename stage will handle it
        }

        /**
         * ISSUE
         */

        RegSlot buildIssue(SyncMeta& syncMeta, BroadCast& bc) override{

            /*
             *  the required Idx
             */
            mWire(checkIdx, _table.getSufficientIdxSize(false));
            zif(e0.sValid){ ///  there is empty room
                zif((b1.sIdx == 0) && (e1.sIdx == (_table.getNumRow()-1))){ ///// zero bubble
                    checkIdx = (e0.sIdx + 1);
                }zelse{ checkIdx = b1.sIdx; }
            }zelse{
                checkIdx = allocPtr;
            }

            /**
             * the issue block
             */
            RegSlot resultRegSlot(_meta);
            WireSlot iw(_table[checkIdx].v());

            cwhile(true){
                zyncc(syncMeta, slotReady(iw)){
                    resultRegSlot <<= iw;
                    tryOwSpecBit(resultRegSlot, iw, bc);
                    //////// reset the table
                    onIssue(checkIdx); //// reset busy

                }
            }
            return resultRegSlot;
        }

    };


}

#endif //KATHRYN_SRC_EXAMPLE_O3_IRSV_H
