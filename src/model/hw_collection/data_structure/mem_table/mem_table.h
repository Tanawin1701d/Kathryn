//
// Created by tanawin on 7/12/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLE_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLE_H
#include "string"

#include "model/hw_collection/data_structure/slot/slot_meta.h"
#include "model/hw_collection/data_structure/slot/wire_slot.h"
#include "model/hw_component/mem_block/mem_block.h"
#include "mem_table_slice_agent.h"

namespace kathryn{

    class MemTable{

        SlotMeta _slotMeta;
        std::vector<MemBlock*> _memStorages;

    public:

        MemTable(const SlotMeta& meta, int amt_row, const std::string& prefix_name = "mem_table");

        SlotMeta get_meta() const{
            return _slotMeta;
        }

        WireSlot gen_dyn_wire_slot(Operable* index);

        void do_glob_asm(Slot& rhs, Operable& required_idx, ASM_TYPE asm_type);


    };


}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_MEMTABLE_MEMTABLE_H