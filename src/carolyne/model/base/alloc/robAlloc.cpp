//
// Created by tanawin on 17/2/2025.
//

#include "robAlloc.h"

namespace kathryn::carolyne{


    PopHST& RobAllocCtrl::createRobAllocTunnel(
        DecodeSlotAnalyzer& dec,
        std::vector<int> desOprIdxs){

        /** buildSendingSlot*/
        //// we care only destination register
        crlAss(_pcWidth > 0, "cannot robAlloc with pcWidth <= 0");
        RowMeta reqRowMeta = {{SLOT_F_PC, SLOT_F_ALLOC_ROB_COMMIT_MODE},
                                 {_pcWidth, _robMeta->getRobCommitModeWidth()}};
        for(int idx = 0; idx < desOprIdxs.size(); idx++){
            int idxInDec = desOprIdxs[idx];
            RowMeta oprAllocMeta = getOprAllocMeta(idxInDec, dec.getOprType(idxInDec, false));
            reqRowMeta += oprAllocMeta;
        }
        /** build Recieving slot*/
        //// we care only rob return index
        RowMeta returnRowMeta({SLOT_F_ROB_IDX},{_robMeta->getIndexWidth()});

        PopHST& resultTunnel = createHST(TN_ALLOCROB, reqRowMeta, returnRowMeta);
        return resultTunnel;
    }

    Slot   RobAllocCtrl::createRobAllocReqSlot(
        DecodeSlotAnalyzer& dec,
        std::vector<int> desOprIdxs,
        std::vector<Slot> renamedPhyRegSlots, ///// the renamed of slot from opr
        Slot pcSlot,
        const Slot& robCommitMode){
        /** buildSendingSlot*/
        //// we care only destination register
        Slot resultSlot = pcSlot + robCommitMode;
        for(int idx = 0; idx < desOprIdxs.size(); idx++){
            Slot oprAllocSlot = createOprAllocSlot(dec, desOprIdxs[idx], renamedPhyRegSlots[idx]);
            resultSlot = resultSlot + oprAllocSlot;
        }
        return resultSlot;
    }

    std::string RobAllocCtrl::getValidOprField(int idx){
        return std::string(SLOT_F_ALLOC_VALID) + "_" + std::to_string(idx);
    }

    std::string RobAllocCtrl::getArcRegField(int idx){
        return std::string(SLOT_F_ALLOC_ARCHID) + "_" + std::to_string(idx);
    }
    std::string RobAllocCtrl::getPhyRegField(int idx){
        return std::string(SLOT_F_ALLOC_PHYID) + "_" + std::to_string(idx);
    }


    RowMeta RobAllocCtrl::getOprAllocMeta(int idx, OprTypeBase* oprType){
        ArchRegFileUTM_Base* archRegFile = oprType->getAllocInfo().relatedArchRegFile;
        PhyRegFileUTM_Base*  phyRegFile  = oprType->getAllocInfo().relatedPhyRegFile;
        /////////////test
        RowMeta oprRobAllocMeta({getValidOprField(idx), getArcRegField(idx)          , getPhyRegField(idx)},
                                {1                  ,  archRegFile->getIndexWidth(), phyRegFile->getIndexWidth()});
        return oprRobAllocMeta;
    }

    Slot RobAllocCtrl::createOprAllocSlot(DecodeSlotAnalyzer& dec, int idx, const Slot& renamedphyReg){
        OprTypeBase* oprTypeBase = dec.getOprType(idx, false);
        crlAss(oprTypeBase != nullptr, "cannot getAllocSlot from null oprType Base");
        RowMeta oprAllocMeta = getOprAllocMeta(idx, oprTypeBase);

        /////// for archReg we want to make it transfer
        Slot oprValidSlot = dec.getOprValidData(idx, false);
        Slot oprRawSlot = dec.getOprRawData(idx, false);
        /////// for phyReg we want it to track
        /// phyRegSlot
        Slot result = oprValidSlot + oprRawSlot + renamedphyReg;
        result.setNewRowMeta(oprAllocMeta);
        return result;
    }




}