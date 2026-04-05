//
// Created by tanawin on 16/9/25.
//

#include "tableSliceAgent.h"
#include "table.h"

namespace kathryn{

    WireSlot TableSliceAgent::v(){
        if (_oneHotEncMode){
            return _table->gen_dyn_wire_slotOHIdx(_required_idx);
        }
        return _table->gen_dyn_wire_slotBiIdx(_required_idx);
    }

    void TableSliceAgent::doGlobAsm(Slot& srcSlot){
        _table->doGlobAsm(srcSlot, _required_idx, ASM_DIRECT, _oneHotEncMode);
    }

    TableSliceAgent& TableSliceAgent::operator <<= (Slot& rhs){
        doGlobAsm(rhs);
        return *this;
    }

    TableSliceAgentDouble TableSliceAgent::operator () (int idx){
        return TableSliceAgentDouble(_table, this, nullptr, idx);
    }

    TableSliceAgentDouble TableSliceAgent::operator () (const std::string& name){
        int idx = _table->getMeta().getIdxPtr(name);
        return TableSliceAgentDouble(_table, this, nullptr, idx);
    }

    TableSliceAgentDouble TableSliceAgent::operator [] (Operable& requiredIdx){
        return TableSliceAgentDouble(_table, this, &requiredIdx, -1);
    }






}