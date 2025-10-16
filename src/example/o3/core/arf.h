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

    struct CommitCmd{

        expression comEn    { 1        };
        expression comRrfPtr{ RRF_SEL  };
        expression comArcIdx{ REG_SEL  };
        expression data     { DATA_LEN };
    };

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


    ////// | rename <-> commit <-> success | missPredict
    ////// mispredict copy the fix table to all table (master table include)
    ////// rename on all table that is free and master table (have the most priorty)
    ////// success copy the master to the success table (rename cannot occur at the same time with rename)
    ////// commit update all table that each element is busy except success table must use with after fixed table


    struct Arf{

        ///////// recoverier
        Table    busy, rename; ////// row re recover tag col is reg
        RegSlot  busyMaster, renameMaster;
        ///////// architecture register actual data
        RegSlot  archRegs;  ////// register architecture register
        ///////// unfortunately commit can happend at the same time with renaming
        CommitCmd commitCmds[2];

        Arf():
            busy        (smARFBusy, SPECTAG_LEN),
            rename      (smARFRenamed, SPECTAG_LEN),
            busyMaster  (smARFBusy),
            renameMaster(smARFRenamed),
            archRegs    (smARFData){

            ///////// reset busy field
            busy      .makeResetEvent(0);
            busyMaster.makeResetEvent(0);
            ///////// reset rename field
            renameMaster.makeResetEvent(0);
            rename      .makeResetEvent(0);

            dataStructProbGrp.arfBusy.init(&busy);
            dataStructProbGrp.arfRename.init(&rename);

        }

        RenamedData getRenamedData(opr& archIdx){
            return {busyMaster  [archIdx].v(),
                    renameMaster[archIdx].v()};
        }

        opr& getArfData(opr& archIdx){
            return archRegs[archIdx].v();
        }

        void updateArfReg(int idx){
            CommitCmd& comCmd = commitCmds[idx];
            zif(comCmd.comEn){
                archRegs[comCmd.comArcIdx] <<= comCmd.data;
            }
        }

        void updateArfRegs(){
            updateArfReg(0);
            updateArfReg(1);
        }

        void setCommitCmd(int idx,
                          opr& comEn,
                          opr& comRrfPtr,
                          opr& comArcIdx,
                          opr& data){

            commitCmds[idx].comEn       = comEn;
            commitCmds[idx].comRrfPtr   = comRrfPtr;
            commitCmds[idx].comArcIdx   = comArcIdx;
            commitCmds[idx].data        = data;

        }

        /**
         *
         * augment temporal data
         *
         */

        /////// they commit all table even though it is branch table
        void augmentTableInCommit(WireSlot& desBusy, WireSlot& desRename, int idx){
            CommitCmd& comCmd = commitCmds[idx];
            zif (comCmd.comEn){
                zif(comCmd.comRrfPtr == desRename[comCmd.comArcIdx].v()){
                    desBusy[comCmd.comArcIdx] = 0;
                }
            }
        }

        void augmentTableInCommits(WireSlot& desBusy, WireSlot& desRename){
            augmentTableInCommit(desBusy, desRename, 0);
            augmentTableInCommit(desBusy, desRename, 1);
        }

        //////// it renames all table except for the branch occupied table

        void augmentTableInRename(WireSlot& desBusy, WireSlot& desRename, RenameCmd& renCmd){
            ///// it rename but rename to 0
            ///// it must set to 0
            ////desBusy[renCmd.renArcIdx] = 0;
            zif(renCmd.renEn){
                desBusy   [renCmd.renArcIdx] = 1;
                desRename [renCmd.renArcIdx] = renCmd.renRrfPtr;
            }
        }


        void onMisPred(opr& misTag){
            SET_ASM_PRI_TO_MANUAL(ARF_MIS_PRIORITY);
            //////
            ////// copy the recovery slot and fill to all table and master
            ////// rcv = recovery slot
            WireSlot rcvbusy  (busy  [OH(misTag)].v());
            WireSlot rcvRename(rename[OH(misTag)].v());

            for(int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                //// busy copy
                busy  (specIdx) <<= rcvbusy;
                rename(specIdx) <<= rcvRename;
            }
            ////// copy to master
            busyMaster   <<= rcvbusy;
            renameMaster <<= rcvRename;

            SET_ASM_PRI_TO_AUTO();
        }

        void onSucPred(opr& fixTag, opr& sucTag){
            ///// commit can occur at the same time
            ///// copy the
            SET_ASM_PRI_TO_MANUAL(ARF_SUC_PRIORITY);
            ///// we have to specify the free tag
            opr& fixTagWOCur = fixTag & ~sucTag; //// curtag is suctag (and we want it to be update)
            OH freeTag(~fixTagWOCur);

            ////// eliminate the data
            WireSlot nextBusy(busyMaster);
            WireSlot nextRename(renameMaster);
            augmentTableInCommits(nextBusy, nextRename);
            ////// fill it reconfig the master
            //OH replacingIdx = broadCast.getSMtag();
            busyMaster      <<= nextBusy;
            renameMaster    <<= nextRename;
            busy  [freeTag] <<= nextBusy;
            rename[freeTag] <<= nextRename;

            SET_ASM_PRI_TO_AUTO();
            ////// the other spectag that going to commit will be done by onCommit
        }

        void onRename(RenameCmd& renCmd1, RenameCmd& renCmd2, Mpft& mpft){
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);

            WireSlot nextmasterBusy  (busyMaster);
            WireSlot nextmasterRename(renameMaster);

            augmentTableInRename(nextmasterBusy, nextmasterRename, renCmd1);

            WireSlot nextmasterBusy2  (nextmasterBusy);
            WireSlot nextmasterRename2(nextmasterRename);
            augmentTableInRename(nextmasterBusy2, nextmasterRename2, renCmd2);


            for (int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                opr& isUsed = mpft.isUsed(specIdx);
                zif(~isUsed){
                    opr& isNewlyUsed1 = renCmd1.isBranch & renCmd1.specTag.sl(specIdx);
                    opr& isNewlyUsed2 = renCmd2.isBranch & renCmd2.specTag.sl(specIdx);
                    zif (isNewlyUsed1){
                        busy  (specIdx) <<= nextmasterBusy;
                        rename(specIdx) <<= nextmasterRename;
                    }zelse{
                        busy  (specIdx) <<= nextmasterBusy2;
                        rename(specIdx) <<= nextmasterRename2;
                    }
                }
            }

            busyMaster   <<= nextmasterBusy2;
            renameMaster <<= nextmasterRename2;
            SET_ASM_PRI_TO_AUTO();
        }

        void onCommit(opr& comEn1    , opr& comRrfPtr1,
                      opr& comArcIdx1, opr& comData1  ,
                      opr& comEn2    , opr& comRrfPtr2,
                      opr& comArcIdx2, opr& comData2)


        {
            ////// not cycle consider (just assign the static assignment)
            setCommitCmd(0, comEn1, comRrfPtr1,
                         comArcIdx1, comData1);

            setCommitCmd(1, comEn2, comRrfPtr2,
                        comArcIdx2, comData2);

            ///// the free spectag should be update
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER);

            ///// check every table
            ///// if success the onSucessPredict will overide it
            for(int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                WireSlot nextBusy(busy(specIdx));
                WireSlot nextRename(rename(specIdx));
                augmentTableInCommits(nextBusy, nextRename);
                busy(specIdx) <<= nextBusy;
                rename(specIdx) <<= nextRename;
            }

            WireSlot nextmasterBusy  (busyMaster);
            WireSlot nextmasterRename(renameMaster);
            augmentTableInCommits(nextmasterBusy, nextmasterRename);
            busyMaster   <<= nextmasterBusy;
            renameMaster <<= nextmasterRename;
            ///// update the data to the table
            updateArfRegs();
            SET_ASM_PRI_TO_AUTO();
        }


        ////// | rename <-> commit <-> success | missPredict
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ARF_H
