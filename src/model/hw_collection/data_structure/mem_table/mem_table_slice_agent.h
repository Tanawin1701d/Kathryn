//
// Created by tanawin on 7/12/25.
//

#ifndef MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLESLICEAGENT_H
#define MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLESLICEAGENT_H
#include "model/hw_collection/data_structure/slot/slot.h"
#include "model/hw_collection/data_structure/slot/wire_slot.h"

namespace kathryn{

    class MemTable;
    class MemTableSliceAgent{

        MemTable* _master_mem_table = nullptr;
        Operable* _required_idx = nullptr;

        MemTableSliceAgent(MemTable* master_mem_table, Operable* required_idx);

        MemTableSliceAgent& operator <<= (Slot& rhs_slot);
        MemTableSliceAgent& operator =   (WireSlot& rhs_wire_slot);
        WireSlot            v();


    };

}

#endif //MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLESLICEAGENT_H