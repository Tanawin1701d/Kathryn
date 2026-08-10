//
// Created by tanawin on 28/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ARF_H
#define KATHRYN_SRC_EXAMPLE_O3_ARF_H

#include "kathryn.h"
#include "slotParam.h"
#include "mpft.h"

namespace kathryn::o3{

    static int ARF_MIS_PRIORITY = DEFAULT_UE_PRI_USER + 4;   ///PARAM ARF
    static int ARF_SUC_PRIORITY = DEFAULT_UE_PRI_USER + 3;   ///PARAM ARF
    static int ARF_REN_PRIORITY = DEFAULT_UE_PRI_USER + 2;   ///PARAM ARF
    static int ARF_COM_PRIORITY = DEFAULT_UE_PRI_USER + 1;   ///PARAM ARF

    struct RenameCmd{   ///MD ARF
        opr& renEn;       // 1   ///CTRL_HC ARF
        opr& renRrfPtr;   // RRF_SEL   ///CTRL_HC ARF
        opr& renArcIdx;   // REG_SEL   ///DATA_HC ARF
        opr& isBranch;   ///CTRL_HC ARF
        opr& specTag;   ///CTRL_HC ARF
    };

    struct RenamedData{   ///MD ARF
        opr& busy;   ///CTRL_HC ARF
        opr& rrfIdx;   ///CTRL_HC ARF
    };

    struct PreRenGrp{   ///MD ARF
        int idx = -1; //// -1 is for master   ///HLH ARF
        mExpr(isAsRecvGrp, 1);   ///CTRL_HWD ARF
        WireSlot busyTemp  {smARFBusy};   ///CTRL_HWD ARF
        WireSlot renameTemp{smARFRenamed};   ///CTRL_HWD ARF


        /**
        *
        * for master table
        */
        void tiedToMaster(RegSlot& busySlot, RegSlot& renameSlot){   ///CTRL_HC ARF
            busyTemp      = busySlot;   ///CTRL_DT ARF
            renameTemp    = renameSlot;   ///CTRL_DT ARF
            busySlot    <<= busyTemp;   ///CTRL_DT ARF
            renameSlot  <<= renameTemp;   ///CTRL_DT ARF
        }

        /**
         * for rcv table
         */

        ///// in normal case loop tied it to the system
        void tiedToTable(Table& busyTable, Table& renameTable){   ///CTRL_HC ARF
            assert(idx != -1);
            busyTemp           = busyTable(idx);   ///CTRL_DT ARF
            renameTemp         = renameTable(idx);   ///CTRL_DT ARF
            busyTable(idx)   <<= busyTemp;   ///CTRL_DT ARF
            renameTable(idx) <<= renameTemp;   ///CTRL_DT ARF
        }

        void onMisPred(opr&      misTag,                 ///CTRL_HC ARF
                       Table&    rcvTabBusy,             ///CTRL_HC ARF
                       Table&    rcvTabRename){          ///CTRL_HC ARF
            SET_ASM_PRI_TO_MANUAL(ARF_MIS_PRIORITY);     ///CTRL_CL ARF
            busyTemp   = rcvTabBusy  [OH(misTag)].v();   ///CTRL_DT ARF
            renameTemp = rcvTabRename[OH(misTag)].v();   ///CTRL_DT ARF
            SET_ASM_PRI_TO_AUTO();                       ///CTRL_CL ARF
        }

        /////// system going to succcess
        void onSucPred(opr& sucTag, PreRenGrp& masterRenGrp){   ///CTRL_HC ARF
            SET_ASM_PRI_TO_MANUAL(ARF_SUC_PRIORITY);            ///CTRL_CL ARF
            zif(sucTag.sl(idx) | (~isAsRecvGrp)){               ///CTRL_CL ARF
                busyTemp   = masterRenGrp.busyTemp;             ///CTRL_DT ARF
                renameTemp = masterRenGrp.renameTemp;           ///CTRL_DT ARF
            }
            SET_ASM_PRI_TO_AUTO();                              ///CTRL_CL ARF
        }

        void commitBase(opr& comEn    , opr& comRrfPtr,         ///CTRL_HC ARF
                        opr& comArcIdx, RegSlot& renameBase){   ///CTRL_HC+DATA_HC ARF

            ///// busy doesnt have to be set if it unset already, it is ok!
            opr& comEntryMatch =  (renameBase[comArcIdx].v() == comRrfPtr);   ///CTRL_CL ARF
            zif(comEn && comEntryMatch){                                      ///CTRL_CL ARF
                busyTemp[comArcIdx] = 0;                                      ///CTRL_DT ARF
            }

        }

        void onCommit(opr& comEn1    , opr& comRrfPtr1,   ///CTRL_HC ARF
                      opr& comArcIdx1,   ///DATA_HC ARF
                      opr& comEn2    , opr& comRrfPtr2,   ///CTRL_HC ARF
                      opr& comArcIdx2,   ///DATA_HC ARF
                      RegSlot& renameReg){   ///CTRL_HC ARF
            SET_ASM_PRI_TO_MANUAL(ARF_COM_PRIORITY);   ///CTRL_CL ARF
            commitBase(comEn1, comRrfPtr1, comArcIdx1, renameReg);   ///CTRL_HC+DATA_HC ARF
            commitBase(comEn2, comRrfPtr2, comArcIdx2, renameReg);   ///CTRL_HC+DATA_HC ARF
            SET_ASM_PRI_TO_AUTO();   ///CTRL_CL ARF
        }

        void renameBase(RenameCmd& renCmd){                        ///CTRL_HC+DATA_HC ARF
            zif(renCmd.renEn){                                     ///CTRL_CL ARF
                busyTemp[renCmd.renArcIdx]   = 1;                  ///CTRL_DT ARF
                renameTemp[renCmd.renArcIdx] = renCmd.renRrfPtr;   ///CTRL_DT ARF
            }
        }

        void onRename(RenameCmd& renCmd1, RenameCmd& renCmd2, bool override = false){   ///CTRL_HC+DATA_HC ARF
            SET_ASM_PRI_TO_MANUAL(ARF_REN_PRIORITY);   ///CTRL_CL ARF
            if (override){   ///HLH ARF
                renameBase(renCmd1);              //// order cannot be changed   ///CTRL_HC+DATA_HC ARF
                renameBase(renCmd2);                                             ///CTRL_HC+DATA_HC ARF
            }else{
                ////// the isAsRecvGrp is set from decode stage
                ////// it should be undone first
                opr& instr1WantThisSlotToRcv = (renCmd1.isBranch && renCmd1.specTag.sl(idx));   ///CTRL_CL ARF
                opr& instr2WantThisSlotToRcv = (renCmd2.isBranch && renCmd2.specTag.sl(idx));   ///CTRL_CL ARF
                ////// undo both instruction 1 and instruction 2
                opr& isAsRecvGrp_undo = (isAsRecvGrp &&                                         ///CTRL_CL ARF
                                         (!(instr1WantThisSlotToRcv |                           ///CTRL_CL ARF
                                            instr2WantThisSlotToRcv))                           ///CTRL_CL ARF
                                         );
                ////// rename 1
                zif(~isAsRecvGrp_undo){    ///CTRL_CL ARF
                    renameBase(renCmd1);   ///CTRL_HC+DATA_HC ARF
                }
                ////// rename 2 (if the first instruction is branch and this table is just rcv )
                opr& isAsRecvGrp_undo_only_second_instr = (isAsRecvGrp_undo |  instr1WantThisSlotToRcv);   ///CTRL_CL ARF
                zif (~isAsRecvGrp_undo_only_second_instr){                                                 ///CTRL_CL ARF
                    renameBase(renCmd2);                                                                   ///CTRL_HC+DATA_HC ARF
                }
            }
            SET_ASM_PRI_TO_AUTO();   ///CTRL_CL ARF
        }

    };


    ////// | rename <-> commit <-> success | missPredict
    ////// mispredict copy the fix table to all table (master table include)
    ////// rename on all table that is free and master table (have the most priorty)
    ////// success copy the master to the success table (rename cannot occur at the same time with rename)
    ////// commit update all table that each element is busy except success table must use with after fixed table


    struct Arf{   ///MD ARF

        ////// rename table
        Table    busy        {smARFBusy, SPECTAG_LEN};      ///CTRL_HWD ARF
        Table    rename      {smARFRenamed, SPECTAG_LEN};   ///CTRL_HWD ARF
        RegSlot  busyMaster  {smARFBusy};                   ///CTRL_HWD ARF
        RegSlot  renameMaster{smARFRenamed};                ///CTRL_HWD ARF
        PreRenGrp preRenGrp[SPECTAG_LEN];                   ///MD ARF
        PreRenGrp preRenMaster;                             ///MD ARF
        ////// architecture data file
        RegSlot  archRegs    {smARFData};                   ///DATA_HWD ARF

        explicit Arf(Mpft& mpft){                       ///CTRL_HC ARF
            ////// reset the register
            busy        .makeResetEvent(0);             ///CTRL_DT ARF
            rename      .makeResetEvent(0);             ///CTRL_DT ARF
            busyMaster  .makeResetEvent(0);             ///CTRL_DT ARF
            renameMaster.makeResetEvent(0);             ///CTRL_DT ARF
            dataStructProbGrp.arfBusy.init(&busy);      ///DC
            dataStructProbGrp.arfRename.init(&rename);  ///DC

            ////// initialize preRenGrp
            for(int i = 0; i < SPECTAG_LEN; i++){          ///HLH ARF
                preRenGrp[i].idx = i;                      ///HLH ARF
                preRenGrp[i].isAsRecvGrp = mpft.isUsed(i); ///CTRL_DT ARF
                preRenGrp[i].tiedToTable(busy, rename);    ///CTRL_HC ARF
            }
            preRenMaster.idx         = -1;                         ///HLH ARF
            preRenMaster.isAsRecvGrp =  1;                         ///CTRL_DT ARF
            preRenMaster.tiedToMaster(busyMaster, renameMaster);   ///CTRL_HC ARF
        }

        RenamedData getRenamedData(opr& archIdx){   ///DATA_HC ARF
            return {busyMaster  [archIdx].v(),   ///CTRL_HC ARF
                    renameMaster[archIdx].v()};   ///CTRL_HC ARF
        }

        opr& getArfData(opr& archIdx){   ///DATA_HC ARF
            return archRegs[archIdx].v();   ///DATA_HC ARF
        }

        void updateArfReg(opr& comEn, opr& comArcIdx, opr& data){   ///CTRL_HC+DATA_HC ARF
            zif(comEn & (comArcIdx != 0)){   ///CTRL_CL ARF
                archRegs[comArcIdx] <<= data;   ///DATA_DT ARF
            }
        }
        void onMisPred(opr& misTag){   ///CTRL_HC ARF
            for(int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){   ///HLH ARF
                preRenGrp[specIdx].onMisPred(misTag, busy, rename);   ///CTRL_HC ARF
            }
            preRenMaster.onMisPred(misTag, busy, rename);   ///CTRL_HC ARF
        }

        void onSucPred(opr& sucTag){   ///CTRL_HC ARF
            for (int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){   ///HLH ARF
                ///////// it must be data from preMaster because it can be occur with commit at the same time
                preRenGrp[specIdx].onSucPred(sucTag, preRenMaster);   ///CTRL_HC ARF
            }
        }

        void onRename(RenameCmd& renCmd1, RenameCmd& renCmd2){   ///CTRL_HC+DATA_HC ARF
            for (int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){   ///HLH ARF
                preRenGrp[specIdx].onRename(renCmd1, renCmd2, false);   ///CTRL_HC+DATA_HC ARF
            }
            preRenMaster.onRename(renCmd1, renCmd2, true);   ///CTRL_HC+DATA_HC ARF
        }

        void onCommit(opr& comEn1    , opr& comRrfPtr1,   ///CTRL_HC ARF
                      opr& comArcIdx1, opr& comData1  ,   ///DATA_HC ARF
                      opr& comEn2    , opr& comRrfPtr2,   ///CTRL_HC ARF
                      opr& comArcIdx2, opr& comData2)   ///DATA_HC ARF
        {
            for (int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){   ///HLH ARF
                preRenGrp[specIdx].onCommit(comEn1, comRrfPtr1, comArcIdx1,   ///CTRL_HC+DATA_HC ARF
                                            comEn2, comRrfPtr2, comArcIdx2, ///CTRL_HC+DATA_HC ARF
                                            rename(specIdx)); ///CTRL_HC ARF
            }
            preRenMaster.onCommit(comEn1, comRrfPtr1, comArcIdx1,   ///CTRL_HC+DATA_HC ARF
                                  comEn2, comRrfPtr2, comArcIdx2, ///CTRL_HC+DATA_HC ARF
                                  renameMaster); ///CTRL_HC ARF

            ////// does not need to update any priority//// order cannot be changed
            updateArfReg(comEn1, comArcIdx1, comData1);  //  due to it contain commit eneable at the destination   ///CTRL_HC+DATA_HC ARF
            updateArfReg(comEn2, comArcIdx2, comData2);   ///CTRL_HC+DATA_HC ARF

        }


        ////// | rename <-> commit <-> success | missPredict
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ARF_H
