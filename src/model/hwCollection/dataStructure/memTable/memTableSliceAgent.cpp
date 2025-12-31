//
// Created by tanawin on 7/12/25.
//

#include "memTableSliceAgent.h"
#include "memTable.h"


namespace kathryn{


    MemTableSliceAgent::MemTableSliceAgent(MemTable* masterMemTable, Operable* requiredIdx):
    _masterMemTable(masterMemTable),
    _requiredIdx(requiredIdx){
        assert(_masterMemTable != nullptr);
        assert(_requiredIdx != nullptr);
    }

    MemTableSliceAgent& MemTableSliceAgent::operator<<=(Slot& rhsSlot){
        _masterMemTable->doGlobAsm(rhsSlot, *_requiredIdx, ASM_DIRECT);
        return *this;
    }

    MemTableSliceAgent& MemTableSliceAgent::operator=(WireSlot& rhsWireSlot){
        _masterMemTable->doGlobAsm(rhsWireSlot, *_requiredIdx, ASM_DIRECT);
        return *this;
    }

    WireSlot MemTableSliceAgent::v(){
        assert(_requiredIdx != nullptr);
        return _masterMemTable->genDynWireSlot(_requiredIdx);
    }




}