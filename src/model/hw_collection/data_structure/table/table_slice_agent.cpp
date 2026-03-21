//
// Created by tanawin on 16/9/25.
//

#include "table_slice_agent.h"
#include "table.h"

namespace kathryn{

    WireSlot TableSliceAgent::v(){
        if (_oneHotEncMode){
            return _table->gen_dyn_wire_slotOHIdx(_required_idx);
        }
        return _table->gen_dyn_wire_slotBiIdx(_required_idx);
    }

    void TableSliceAgent::do_glob_asm(Slot& src_slot){
        _table->do_glob_asm(src_slot, _required_idx, ASM_DIRECT, _oneHotEncMode);
    }

    TableSliceAgent& TableSliceAgent::operator <<= (Slot& rhs){
        do_glob_asm(rhs);
        return *this;
    }

    TableSliceAgentDouble TableSliceAgent::operator () (int idx){
        return TableSliceAgentDouble(_table, this, nullptr, idx);
    }

    TableSliceAgentDouble TableSliceAgent::operator () (const std::string& name){
        int idx = _table->get_meta().get_idx_ptr(name);
        return TableSliceAgentDouble(_table, this, nullptr, idx);
    }

    TableSliceAgentDouble TableSliceAgent::operator [] (Operable& required_idx){
        return TableSliceAgentDouble(_table, this, &required_idx, -1);
    }






}