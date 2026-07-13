//
// Created by tanawin on 28/9/25.
//

#ifndef KATHRYN_EXAMPLE_O3_RRF_H
#define KATHRYN_EXAMPLE_O3_RRF_H

#include "kathryn.h"
#include "slotParam.h"


namespace kathryn::o3{

    static int RRF_RENAME_PRI = DEFAULT_UE_PRI_USER + 1;   ///PARAM RRF

    struct PhyEntry{  ///MD RRF
        opr& valid;   ///CTRL_HC RRF
        opr& data;    ///DATA_HC RRF
    };

    struct Rrf{   ///MD RRF

        Table table;                                ///CTRL_HWD+DATA_HWD RRF
        mReg(freenum       , RRF_SEL + 1);          ///CTRL_HWD RRF
        mReg(reqPtr        ,  RRF_SEL);             ///CTRL_HWD RRF
        mReg(nextRrfCycle  , 1);                    ///CTRL_HWD RRF
        mWire(renameReqSize, 2);                    ///CTRL_HWD RRF
        mWire(commitReqSize, 2);                    ///CTRL_HWD RRF
        mVal(RRF_ENTRIES   , RRF_SEL + 1, RRF_NUM); ///PARAM RRF


        Rrf():                                              ///HLH RRF
        table(smRRF, RRF_NUM){                              ///CTRL_HWD+DATA_HWD RRF
            table        .makeColResetEvent(rrfValid, 0);   ///CTRL_DT RRF
            freenum      .makeResetEvent(RRF_NUM);          ///CTRL_DT RRF
            reqPtr       .makeResetEvent();                 ///CTRL_DT RRF
            nextRrfCycle .makeResetEvent();                 ///CTRL_DT RRF
            nextRrfCycle .makeDefEvent();                   ///CTRL_DT RRF

            dataStructProbGrp.rrf.init(&table); ///DC
        }

        opr& isRenamable(opr& req2){                                  ///CTRL_HC RRF
            return (freenum + commitReqSize) >= (req2.uext(2) + 1);   ///CTRL_CL RRF
        }

        Operable& getReqPtr(){return reqPtr;}   ///CTRL_HC RRF

        PhyEntry getPhyEntry(opr& rrfIdx){      ///CTRL_HC RRF
            auto agent = table[rrfIdx];         ///DATA_DT RRF
            return {agent(rrfValid).v(),        ///CTRL_HC RRF
                    agent(rrfData ).v()};       ///DATA_HC RRF
        }

        opr& getPhyData(opr& rrfIdx){           ///CTRL_HC RRF
            return table[rrfIdx](rrfData).v();  ///DATA_HC RRF
        }

        //// fixRrf is the idx who must walk away the size support to be rrf
        void onMisPred(opr& misRrf, opr& curCommitPtr){ /// size supposed to be equal to rrf   ///CTRL_HC RRF
            opr& nextRrf = misRrf + 1;                                                         ///CTRL_CL RRF
            reqPtr <<= nextRrf;                                                                ///CTRL_DT RRF

            freenum <<= (RRF_ENTRIES - nextRrf) + curCommitPtr;           ///CTRL_CL RRF
            zif(curCommitPtr >= nextRrf){                                 ///CTRL_CL RRF
                ///// the free entry is bubble
                /// it must be >= because should be full only (freenum = 0)
                freenum <<= (curCommitPtr - nextRrf).uext(RRF_SEL + 1);   ///CTRL_CL RRF
            }
        }

        /////// unfortunately rename and commit can be occur at the same time
        void doRenameOrCommit(){                                     ///HLH RRF
            freenum <<= (freenum + commitReqSize - renameReqSize);   ///CTRL_CL RRF
        }

        ////// on the table there should no conflict (rename<->wb<->commit)
        ////////////////it is ok to not have req1
        void onRename(opr& req2){                    ///CTRL_HC RRF
            ////// isRenamable must be use
            renameReqSize = req2.uext(2) + 1;        ///CTRL_CL RRF
            doRenameOrCommit();   ///CTRL_CL RRF
            ////// rename have more priority than write back
            SET_ASM_PRI_TO_MANUAL(RRF_RENAME_PRI);   ///CTRL_CL RRF

            table[reqPtr](rrfValid) <<= 0;           ///CTRL_DT RRF
            zif(req2){   ///CTRL_CL RRF
                table[reqPtr+1](rrfValid) <<= 0;     ///CTRL_DT RRF
                ///// request 2 will not set if req1 is set
            }
            opr& nextRrf = reqPtr + renameReqSize;   ///CTRL_CL RRF
            nextRrfCycle <<= reqPtr > nextRrf;       ///CTRL_CL RRF
            reqPtr <<= nextRrf;                      ///CTRL_DT RRF
            SET_ASM_PRI_TO_AUTO();                   ///CTRL_CL RRF

        }

        ////// write back and rename cannot disable the same valid register at the same time
        void onWback(opr& wbPtr, opr& wbData){      ///CTRL_HC+DATA_HC RRF
            auto agent = table[wbPtr];              ///DATA_DT RRF
            agent(rrfValid) <<= 1;                  ///CTRL_DT RRF
            agent(rrfData ) <<= wbData;             ///DATA_DT RRF
        }

        ///// it
        std::pair<opr&, opr&> onCommit(opr& comPtr, opr& com1Cond, opr& com2Cond){   ///CTRL_HC RRF
            ///// com1Cond is only raw condition in the reorder buffer we must check that it is valid or not
            ///// com2Cond
            mWire(com1Avail, 1);                                                            ///CTRL_HWD RRF
            mWire(com2Avail, 1);                                                            ///CTRL_HWD RRF
            opr& roundOver = (reqPtr < comPtr) | (freenum == 0); //// no free turn around   ///CTRL_CL RRF
            opr& rem = gr(roundOver, reqPtr) - comPtr; //// rem means remain to commit      ///CTRL_CL RRF
            com1Avail = rem > 0; //// unequal to 0                                          ///CTRL_CL RRF
            com2Avail = rem > 1; //// unequal to 1 and 0                                    ///CTRL_CL RRF

            //////          resource in rrf(are there actually) & comCondition(finish)
            opr& resCom1 = com1Avail&com1Cond;                                        ///CTRL_CL RRF
            opr& resCom2 = resCom1&com2Avail&com2Cond;                                ///CTRL_CL RRF
            commitReqSize = resCom1.uext(2) + resCom2.uext(2);                        ///CTRL_CL RRF
            doRenameOrCommit(); ////// rename and commit can occur at the same time   ///CTRL_CL RRF
            return {resCom1, resCom2};                                                ///CTRL_HC RRF
        }


    };

}

#endif //KATHRYN_EXAMPLE_O3_RRF_H
