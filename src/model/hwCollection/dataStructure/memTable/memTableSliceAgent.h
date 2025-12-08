//
// Created by tanawin on 7/12/25.
//

#ifndef MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLESLICEAGENT_H
#define MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLESLICEAGENT_H
#include "model/hwCollection/dataStructure/slot/slot.h"
#include "model/hwCollection/dataStructure/slot/wireSlot.h"

namespace kathryn{

    class MemTable;
    class MemTableSliceAgent{

        MemTable* _masterMemTable = nullptr;
        Operable* _requiredIdx = nullptr;

        MemTableSliceAgent(MemTable* masterMemTable, Operable* requiredIdx);

        MemTableSliceAgent& operator <<= (Slot& rhsSlot);
        MemTableSliceAgent& operator =   (WireSlot& rhsWireSlot);
        WireSlot            v();


    };

}

#endif //MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLESLICEAGENT_H