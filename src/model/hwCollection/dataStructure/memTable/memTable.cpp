//
// Created by tanawin on 7/12/25.
//

#include "memTable.h"


namespace kathryn{

    WireSlot MemTable::genDynWireSlot(Operable* index){

        assert(index != nullptr);
        assert(getMeta().getNumField() == _memStorages.size());
        WireSlot resultWireSlot(getMeta());
        /////// recruite opr and assign meta
        std::vector<Operable*> srcOprs;
        for (int colIdx = 0; colIdx < getMeta().getNumField(); colIdx++){
            Operable* colOpr = &((*_memStorages[colIdx])[*index]);
            assert(colOpr != nullptr);
            srcOprs.push_back(colOpr);
        }
        std::vector<AssignMeta*> resultCollector = resultWireSlot.genAssignMetaForAll(srcOprs, ASM_DIRECT);

        /////// assign to build the system
        AsmNode* asmNode = WireSlot::genGrpAsmNode(resultCollector);
        asmNode->dryAssign();
        delete asmNode;
        return resultWireSlot;

    }

    void MemTable::doGlobAsm(Slot& rhs, Operable& requiredIdx, ASM_TYPE asmType){


        SlotMeta srcSlotMeta = rhs.getMeta();

        ///// source index match index, destination index match index
        auto [srcMatchIdxs, desMatchIdxs] = getMeta().matchByName(srcSlotMeta);

        assert(srcMatchIdxs.size() == desMatchIdxs.size());

        std::vector<AssignMeta*> resultCollector;

        for(int idx = 0; idx < srcMatchIdxs.size(); idx++){
            auto srcIdx = srcMatchIdxs[idx];
            auto desIdx = desMatchIdxs[idx];
            /** get src des hw */
            Operable* srcValue = rhs.hwFieldRefAt(srcIdx)._opr;
            MemBlock* targetMemBlk = _memStorages[desIdx];
            MemBlockEleHolder* memEleHolder = &(*targetMemBlk)[requiredIdx];
            /** buidl */
            AssignMeta* assMeta =
            memEleHolder ->
             generateAssignMeta(*srcValue,
                                {0, targetMemBlk->getWidthSize()},
                                asmType,
                                memEleHolder->getCurAssignClkMode()
                               );

            resultCollector.push_back(assMeta);

        }

        auto* asmNode = new AsmNode(resultCollector);
        asmNode->dryAssign();
        delete asmNode;


    }


}