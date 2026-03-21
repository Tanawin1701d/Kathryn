//
// Created by tanawin on 7/12/25.
//

#include "mem_table_slice_agent.h"
#include "mem_table.h"


namespace kathryn{


    MemTableSliceAgent::MemTableSliceAgent(MemTable* master_mem_table, Operable* required_idx):
    _master_mem_table(master_mem_table),
    _required_idx(required_idx){
        assert(_master_mem_table != nullptr);
        assert(_required_idx != nullptr);
    }

    MemTableSliceAgent& MemTableSliceAgent::operator<<=(Slot& rhs_slot){
        _master_mem_table->do_glob_asm(rhs_slot, *_required_idx, ASM_DIRECT);
        return *this;
    }

    MemTableSliceAgent& MemTableSliceAgent::operator=(WireSlot& rhs_wire_slot){
        _master_mem_table->do_glob_asm(rhs_wire_slot, *_required_idx, ASM_DIRECT);
        return *this;
    }

    WireSlot MemTableSliceAgent::v(){
        assert(_required_idx != nullptr);
        return _master_mem_table->gen_dyn_wire_slot(_required_idx);
    }




}