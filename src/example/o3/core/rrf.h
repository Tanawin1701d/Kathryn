//
// Created by tanawin on 28/9/25.
//

#ifndef KATHRYN_EXAMPLE_O3_RRF_H
#define KATHRYN_EXAMPLE_O3_RRF_H

#include "kathryn.h"
#include "slotParam.h"


namespace kathryn::o3{

    static int RRF_RENAME_PRI = DEFAULT_UE_PRI_USER + 1;

    struct PhyEntry{
        opr& valid;
        opr& data;
    };

    struct Rrf{

        Table table;
        mReg(freenum, RRF_SEL + 1);
        mReg(reqPtr,  RRF_SEL);
        mReg(nextRrfCycle, 1);
        mWire(renameReqSize, 2);
        mWire(commitReqSize, 2);
        mVal(RRF_ENTRIES, RRF_SEL + 1, RRF_NUM);


        Rrf():
        table(smRRF, RRF_NUM){
            table       .makeColResetEvent(rrfValid, 0);
            freenum     .makeResetEvent(RRF_NUM);
            reqPtr      .makeResetEvent();
            nextRrfCycle.makeResetEvent();

            dataStructProbGrp.rrf.init(&table);


        }

        opr& isRecur(){ return nextRrfCycle;}

        opr& isRenamable(opr& req2){
            return (freenum + commitReqSize) >= (req2.uext(2) + 1);
        }

        Operable& getReqPtr(){return reqPtr;}

        PhyEntry getPhyEntry(opr& rrfIdx){
            auto agent = table[rrfIdx];
            return {agent(rrfValid).v(),
                    agent(rrfData ).v()};
        }

        opr& getPhyData(opr& rrfIdx){
            return table[rrfIdx](rrfData).v();
        }

        //// fixRrf is the idx who must walk away the size support to be rrf
        void onMisPred(opr& misRrf, opr& curCommitPtr){ /// size supposed to be equal to rrf
            reqPtr <<= misRrf;
            zif(curCommitPtr < misRrf){
                freenum <<= (misRrf - curCommitPtr);
            }zelse{
                freenum <<= (RRF_ENTRIES - curCommitPtr + misRrf);
            }
        }

        /////// unfortunately rename and commit can be occur at the same time
        void doRenameOrCommit(){
            freenum <<= (freenum + commitReqSize - renameReqSize);
        }

        ////// on the table there should no conflict (rename<->wb<->commit)
        void onRename(opr& req1, opr& req2){
            ////// isRenamable must be use
            renameReqSize = req1.uext(2) + req2.uext(2);
            doRenameOrCommit();
            ////// rename have more priority than write back
            SET_ASM_PRI_TO_MANUAL(RRF_RENAME_PRI);
            zif(req1){
                table[reqPtr](rrfValid) <<= 0;
            }
            zif(req2){
                table[reqPtr+1](rrfValid) <<= 0;
                ///// request 2 will not set if req1 is set
            }
            opr& nextRrf = reqPtr + renameReqSize;
            nextRrfCycle <<= reqPtr > nextRrf;
            reqPtr <<= nextRrf;
            SET_ASM_PRI_TO_AUTO();

        }

        ////// write back and rename cannot disable the same valid register at the same time
        void onWback(opr& wbPtr, opr& wbData){
            auto agent = table[wbPtr];
            agent(rrfValid) <<= 1;
            agent(rrfData ) <<= wbData;
        }

        ///// it
        std::pair<opr&, opr&> onCommit(opr& comPtr, opr& com1Cond, opr& com2Cond){
            ///// com1Cond is only raw condition in the reorder buffer we must check that it is valid or not
            ///// com2Cond
            mWire(com1Avail, 1);
            mWire(com2Avail, 1);
            opr& roundOver = (reqPtr < comPtr) | (freenum == 0); //// no free turn around
            opr& rem = gr(roundOver, reqPtr) - comPtr; //// rem means remain to commit
            com1Avail = rem > 0; //// unequal to 0
            com2Avail = rem > 1; //// unequal to 1 and 0

            opr& resCom1 = com1Avail&com1Cond;
            opr& resCom2 = com2Avail&com2Cond;
            commitReqSize = resCom1.uext(2) + resCom2.uext(2);
            doRenameOrCommit(); ////// rename and commit can occur at the same time
            return {resCom1, resCom2};
        }


    };

}

#endif //KATHRYN_EXAMPLE_O3_RRF_H
