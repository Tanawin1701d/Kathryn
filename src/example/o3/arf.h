//
// Created by tanawin on 28/9/25.
//

#ifndef KATHRYN_SRC_EXAMPLE_O3_ARF_H
#define KATHRYN_SRC_EXAMPLE_O3_ARF_H

#include "kathryn.h"
#include "slotParam.h"
#include "stageStruct.h"

namespace kathryn::o3{

    struct CommitCmd{
        mWire(comEn, 1         );
        mWire(comRrfPtr , RRF_SEL );
        mWire(comArcIdx, REG_SEL );
        mWire(data   , DATA_LEN);
    };

    struct RenameCmd{
        mWire(renEn      , 1);
        mWire(renRrfPtr  , RRF_SEL);
        mWire(renArcIdx  , REG_SEL);
        mWire(validvector, SPECTAG_LEN);
    };

    struct RenamedData{
        opr& busy;
        opr& rrfIdx;
    };

    struct Arf{

        Table busy, rename;
        RegSlot  busyMaster, renameMaster;

        RegSlot  archRegs; ////// register architecture register

        CommitCmd commitCmds[2];


        BroadCast& broadCast;

        Arf(BroadCast& bc):
            busy(smARFBusy, SPECTAG_LEN),
            rename(smARFRenamed, SPECTAG_LEN),
            busyMaster(smARFBusy),
            renameMaster(smARFRenamed),
            archRegs(smARFData),
            broadCast(bc){
            busy.doReset();
            rename.doReset();
            busyMaster.doReset();
            renameMaster.doReset();
            archRegs.doReset();
        }




        /////// EXAMPLE
        // void buildArfRenamingLogic(){
        //
        //     zif (){ ///// case miss predict
        //
        //     }zelif(){ ///// case sucess predict
        //
        //     }zelse{
        //         ////// do normal commit
        //
        //     }
        // }

        RenamedData getRenamedData(opr& archIdx){
            return {busyMaster[archIdx].v(),
                renameMaster[archIdx].v()};

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

        void augmentTableInCommit(WireSlot& desBusy, WireSlot& desRename, int idx){

            CommitCmd& comCmd = commitCmds[idx];
            zif (comCmd.comEn){
                zif(comCmd.comRrfPtr == desRename[comCmd.comArcIdx].v()){
                    desBusy[comCmd.comArcIdx] = 1;
                }
            }
        }

        void augmentTableInCommits(WireSlot& desBusy, WireSlot& desRename){
            augmentTableInCommit(desBusy, desRename, 0);
            augmentTableInCommit(desBusy, desRename, 1);
        }

        void augmentTableInRename(WireSlot& desBusy, WireSlot& desRename, RenameCmd& renCmd){
            ///// it rename but rename to 0
            ///// it must set to 0
            desBusy[renCmd.renArcIdx] = 0;
            zif(renCmd.renEn){
                desBusy[renCmd.renArcIdx] = 1;
                desRename[renCmd.renArcIdx] = renCmd.renRrfPtr;
            }
        }


        void onMissPredict(){
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+3);
            ////// copy the recovery table and fill to all table
            ///
            WireSlot rcvbusy  (busy  [OH(broadCast.getSMtag())].v());
            WireSlot rcvRename(rename[OH(broadCast.getSMtag())].v());

            for(int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                //// busy copy
                busy(specIdx) <<= rcvbusy;
                rename(specIdx) <<= rcvRename;
            }
            ////// copy to master
            busyMaster   <<= rcvbusy;
            renameMaster <<= rcvRename;


            SET_ASM_PRI_TO_AUTO();
        }

        void onSucessPredict(){ ///// it must handle case the commit as well
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+2);

            ////// eliminated the data
            WireSlot nextBusy(busyMaster);
            WireSlot nextRename(renameMaster);
            augmentTableInCommits(nextBusy, nextRename);
            ////// fill it reconfig the master
            OH replacingIdx = broadCast.getSMtag();
            busyMaster           <<= nextBusy;
            renameMaster         <<= nextRename;
            busy  [replacingIdx] <<= nextBusy;
            rename[replacingIdx] <<= nextRename;

            SET_ASM_PRI_TO_AUTO();
            ////// the other spectag that going to commit will be done by onCommit
        }

        void onRename(RenameCmd& renCmd1, RenameCmd& renCmd2){
            SET_ASM_PRI_TO_MANUAL(DEFAULT_UE_PRI_USER+1);

            for (int renIdx = 0; renIdx < 2; renIdx++){
                RenameCmd& renCmd = renIdx == 0? renCmd1 : renCmd2;
                /////////// augment in some table
                for (int specIdx = 0; specIdx < SPECTAG_LEN; specIdx++){
                    WireSlot nextBusy(busy(specIdx));
                    WireSlot nextRename(rename(specIdx));
                    zif(renCmd.validvector(specIdx)){
                        augmentTableInRename(nextBusy, nextRename, renCmd);
                        busy(specIdx) <<= nextBusy;
                        rename(specIdx) <<= nextRename;
                    }
                }
                ////// augment master
                WireSlot nextmasterBusy  (busyMaster);
                WireSlot nextmasterRename(renameMaster);
                augmentTableInRename(nextmasterBusy, nextmasterRename, renCmd);
                busyMaster   <<= nextmasterBusy;
                renameMaster <<= nextmasterRename;

            }
            SET_ASM_PRI_TO_AUTO();
        }

        void onCommit(){ ///// the free spectag should be update
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

            SET_ASM_PRI_TO_AUTO();
        }
        ////// | rename <-> commit <-> success | missPredict
    };

}

#endif //KATHRYN_SRC_EXAMPLE_O3_ARF_H
