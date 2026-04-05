//
// Created by tanawin on 7/12/25.
//

#include "memTableSliceAgent.h"
#include "memTable.h"


namespace kathryn{


    MemTableSliceAgent::MemTableSliceAgent(MemTable* masterMemTable, Operable* requiredIdx):
    _master_mem_table(masterMemTable),
    _required_idx(requiredIdx){
        assert(_master_mem_table != nullptr);
        assert(_required_idx != nullptr);
    }

    MemTableSliceAgent& MemTableSliceAgent::operator<<=(Slot& rhsSlot){
        _master_mem_table->doGlobAsm(rhsSlot, *_required_idx, ASM_DIRECT);
        return *this;
    }

    MemTableSliceAgent& MemTableSliceAgent::operator=(WireSlot& rhsWireSlot){
        _master_mem_table->doGlobAsm(rhsWireSlot, *_required_idx, ASM_DIRECT);
        return *this;
    }

    WireSlot MemTableSliceAgent::v(){
        assert(_required_idx != nullptr);
        return _master_mem_table->genDynWireSlot(_required_idx);
    }




}