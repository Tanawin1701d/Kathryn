//
// Created by tanawin on 28/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ARF_H
#define KATHRYN_SRC_EXAMPLE_O3_ARF_H

#include "kathryn.h"
#include "slotParam.h"
#include "mpft.h"

namespace kathryn::o3{

    static int ARF_MIS_PRIORITY = DEFAULT_UE_PRI_USER + 3;
    static int ARF_SUC_PRIORITY = DEFAULT_UE_PRI_USER + 2;
    static int ARF_REN_PRIORITY = DEFAULT_UE_PRI_USER + 1;
    static int ARF_COM_PRIORITY = DEFAULT_UE_PRI_USER;

    struct RenameCmd{
        opr& renEn;       // 1
        opr& renRrfPtr;   // RRF_SEL
        opr& renArcIdx;   // REG_SEL
        opr& isBranch;
        opr& specTag;
    };

    struct RenamedData{
        opr& busy;
        opr& rrfIdx;
    };

    struct PreRenGrp{
        int idx = -1; //// -1 is for master
        mExpr(isAsRecvGrp, 1);
        WireSlot busyTemp  {smARFBusy};
        WireSlot renameTemp{smARFRenamed};


        /**
        *
        * for master table
        */
        void tiedToMaster(RegSlot& busySlot, RegSlot& renameSlot){
            busyTemp      = busySlot;
            renameTemp    = renameSlot;
            busySlot    <<= busyTemp;
            renameSlot  <<= renameTemp;
        }

        /**
         * for rcv table
         */

        ///// in normal case loop tied it to the system
        void tiedToTable(Table& busyTable, Table& renameTable){
            assert(idx != -1);
            busyTemp           = busyTable(idx);
            renameTemp         = renameTable(idx);
            busyTable(idx)   <<= busyTemp;
            renameTable(idx) <<= renameTemp;
        }

        void onMisPred(opr& misTag,
                       Table&    rcvTabBusy,
                       Table&    rcvTabRename){
            SET_ASM_PRI_TO_MANUAL(ARF_MIS_PRIORITY);
            busyTemp   = rcvTabBusy  [OH(misTag)].v();
            renameTemp = rcvTabRename[OH(misTag)].v();
            SET_ASM_PRI_TO_AUTO();
        }

        /////// system going to succcess
        void onSucPred(opr& sucTag, PreRenGrp& masterRenGrp){
            SET_ASM_PRI_TO_MANUAL(ARF_SUC_PRIORITY);
            zif(sucTag.sl(idx) | (~isAsRecvGrp)){
                busyTemp   = masterRenGrp.busyTemp;
                renameTemp = masterRenGrp.renameTemp;
            }
            SET_ASM_PRI_TO_AUTO();
        }

        void commitBase(opr& comEn    , opr& comRrfPtr,
                        opr& comArcIdx, RegSlot& renameBase){

            ///// busy doesnt have to be set if it unset already, it is ok!
            opr& comEntryMatch =  (renameBase[comArcIdx].v() == comRrfPtr);
            zif(comEn && comEntryMatch){
                busyTemp[comArcIdx] = 0;
            }

        }

        void onCommit(opr& comEn1    , opr& comRrfPtr1,
                      opr& comArcIdx1,
                      opr& comEn2    , opr& comRrfPtr2,
                      opr& comArcIdx2,
                      RegSlot& renameReg){
            SET_ASM_PRI_TO_MANUAL(ARF_COM_PRIORITY);
            commitBase(comEn1, comRrfPtr1, comArcIdx1, renameReg);
            commitBase(comEn2, comRrfPtr2, comArcIdx2, renameReg);
            SET_ASM_PRI_TO_AUTO();
        }

        void renameBase(RenameCmd& renCmd){
            zif(renCmd.renEn){
                busyTemp[renCmd.renArcIdx]   = 1;
                renameTemp[renCmd.renArcIdx] = renCmd.renRrfPtr;
            }
        }

        void onRename(RenameCmd& renCmd1, RenameCmd& renCmd2, bool override = false){
            SET_ASM_PRI_TO_MANUAL(ARF_REN_PRIORITY);
            if (override){
                renameBase(renCmd1); //// order cannot be changed
                renameBase(renCmd2);
            }else{
                ////// the isAsRecvGrp is set from decode stage
                ////// it should be undone first
                opr& instr1WantThisSlotToRcv = (renCmd1.isBranch && renCmd1.specTag.sl(idx));
                opr& instr2WantThisSlotToRcv = (renCmd2.isBranch && renCmd2.specTag.sl(idx));
                ////// undo both instruction 1 and instruction 2
                opr& isAsRecvGrp_undo = (isAsRecvGrp &&
                                         (!(instr1WantThisSlotToRcv |
                                            instr2WantThisSlotToRcv))
                                         );
                ////// rename 1
                zif(~isAsRecvGrp_undo){
                    renameBase(renCmd1);
                }
                ////// rename 2 (if the first instruction is branch and this table is just rcv )
                opr& isAsRecvGrp_undo_only_second_instr = (isAsRecvGrp_undo |  instr1WantThisSlotToRcv);
                zif (~isAsRecvGrp_undo_only_second_instr){
                    renameBase(renCmd2);
                }
            }
            SET_ASM_PRI_TO_AUTO();
        }

    };


    ////// | rename <-> commit <-> success | missPredict
    ////// mispredict copy the fix table to all table (master table include)
    ////// rename on all table that is free and master table (have the most priorty)
    ////// success copy the master to the success table (rename cannot occur at the same time with rename)
    ////// commit update all table that each element is busy except success table must use with after fixed table


    struct Arf{

        ////// rename table
        Table    busy        {smARFBusy, SPECTAG_LEN};
        Table    rename      {smARFRenamed, SPECTAG_LEN}; ////// row re recover tag col is reg
        RegSlot  busyMaster  {smARFBusy};
        RegSlot  renameMaster{smARFRenamed};
        PreRenGrp preRenGrp[SPECTAG_LEN];
        PreRenGrp preRenMaster;
        ////// architecture data file
        RegSlot  archRegs    {smARFData};

        explicit Arf(Mpft& mpft){
            ////// reset the register
            busy        .makeResetEvent(0);
            rename      .makeResetEvent(0);
            busyMaster  .makeResetEvent(0);
            renameMaster.makeResetEvent(0);
            dataStructProbGrp.arfBusy.init(&busy);
            dataStructProbGrp.arfRename.init(&rename);

            ////// initialize preRenGrp
            for(int i = 0; i < SPECTAG_LEN; i++){
                preRenGrp[i].idx = i;
                preRenGrp[i].isAsRecvGrp = mpft.isUsed(i);
                preRenGrp[i].tiedToTable(busy, rename);
            }
            preRenMaster.idx         = -1;
            preRenMaster.isAsRecvGrp =  1;
            preRenMaster.tiedToMaster(busyMaster, renameMaster);
        }

        RenamedData getRenamedData(opr& archIdx){
            return {busyMaster  [archIdx].v(),
                    renameMaster[archIdx].v()};
        }

        opr& getArfData(opr& archIdx){
            return archRegs[archIdx].v();
        }

        void updateArfReg(opr& comEn, opr& comArcIdx, opr& data){
            zif(comEn & (comArcIdx != 0)){
                archRegs[comArcIdx] <<= data;
            }
        }
        void onMisPred(opr& misTag){
            for(int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                preRenGrp[specIdx].onMisPred(misTag, busy, rename);
            }
            preRenMaster.onMisPred(misTag, busy, rename);
        }

        void onSucPred(opr& sucTag){
            for (int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                ///////// it must be data from preMaster because it can be occur with commit at the same time
                preRenGrp[specIdx].onSucPred(sucTag, preRenMaster);
            }
        }

        void onRename(RenameCmd& renCmd1, RenameCmd& renCmd2){
            for (int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                preRenGrp[specIdx].onRename(renCmd1, renCmd2, false);
            }
            preRenMaster.onRename(renCmd1, renCmd2, true);
        }

        void onCommit(opr& comEn1    , opr& comRrfPtr1,
                      opr& comArcIdx1, opr& comData1  ,
                      opr& comEn2    , opr& comRrfPtr2,
                      opr& comArcIdx2, opr& comData2)
        {
            for (int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                preRenGrp[specIdx].onCommit(comEn1, comRrfPtr1, comArcIdx1,
                                            comEn2, comRrfPtr2, comArcIdx2,
                                            rename(specIdx));
            }
            preRenMaster.onCommit(comEn1, comRrfPtr1, comArcIdx1,
                                  comEn2, comRrfPtr2, comArcIdx2,
                                  renameMaster);

            ////// does not need to update any priority//// order cannot be changed
            updateArfReg(comEn1, comArcIdx1, comData1);
            updateArfReg(comEn2, comArcIdx2, comData2);

        }


        ////// | rename <-> commit <-> success | missPredict
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ARF_H
