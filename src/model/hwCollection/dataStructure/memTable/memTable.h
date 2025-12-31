//
// Created by tanawin on 7/12/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLE_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLE_H
#include <string>

#include "model/hwCollection/dataStructure/slot/slotMeta.h"
#include "model/hwCollection/dataStructure/slot/wireSlot.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "memTableSliceAgent.h"

namespace kathryn{

    class MemTable{

        SlotMeta _slotMeta;
        std::vector<MemBlock*> _memStorages;

    public:

        MemTable(const SlotMeta& meta, int amtRow, const std::string& prefixName = "memTable");

        SlotMeta getMeta() const{
            return _slotMeta;
        }

        WireSlot genDynWireSlot(Operable* index);

        void doGlobAsm(Slot& rhs, Operable& requiredIdx, ASM_TYPE asmType);


    };


}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLE_H