//
// Created by tanawin on 28/9/25.
//

#ifndef KATHRYN_EXAMPLE_O3_RRF_H
#define KATHRYN_EXAMPLE_O3_RRF_H

#include "kathryn.h"
#include "slotParam.h"

namespace kathryn::o3{

    struct Rrf{

        Table table;
        mReg(freenum, RRF_SEL + 1);
        mReg(reqPtr,  RRF_SEL);
        mWire(renameReqSize, 2);
        mWire(commitReqSize, 2);
        mVal(RRF_ENTRIES, RRF_SEL + 1, RRF_NUM);

        Rrf():
        table(smRRF, RRF_NUM){}


        //// fixRrf is the idx who must walk away the size support to be rrf
        void onMissPred(opr& fixRrf, opr& curCommitPtr){ /// size supposed to be equal to rrf
            reqPtr <<= fixRrf;
            zif(curCommitPtr <= fixRrf){
                freenum <<= (fixRrf - curCommitPtr);
            }zelse{
                freenum <<= (RRF_ENTRIES - curCommitPtr) + fixRrf;
            }
        }

        opr& isRecur(opr& req1, opr& req2){

            return (reqPtr + req1 + req2) <= (reqPtr);
        }

        opr& isRenamable(opr& req1, opr& req2,
                         opr& com1, opr& com2){
            return (freenum + com1 + com2) >= (req1 + req2);
        }

        void doRenameOrCommit(){
            freenum <<= (freenum + commitReqSize - renameReqSize);
        }

        ////// on rename might have conflicted
        void onRename(opr& req1, opr& req2){
            ////// isRenamable must be use
            renameReqSize = req1 + req2;
            doRenameOrCommit();
            zif(req1){
                table[reqPtr](rrfValid) <<= 0;
                reqPtr <<= (reqPtr + 1);
            }
            zif(req2){
                table[reqPtr+1](rrfValid) <<= 0;
                reqPtr <<= (reqPtr + 2);
            }

        }

        ////// write back and rename cannot disable the same valid register at the same time
        void onWback(opr& wbPtr, opr& wbValid, opr& wbData){
            table[wbPtr](rrfValid) <<= wbValid;
            table[wbPtr](rrfData ) <<= wbData;
            ///// to do register the writeback data to the broadcast

        }

        void onCommit(opr& comPtr, opr& com1, opr& com2){
            commitReqSize = commitReqSize + com1 + com2;
            doRenameOrCommit();
        }


    };

}

#endif //KATHRYN_EXAMPLE_O3_RRF_H
