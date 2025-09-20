//
// Created by tanawin on 16/9/25.
//

#include "tableSliceAgentDouble.h"

#include "table.h"
#include "tableSliceAgent.h"

namespace kathryn{

    TableSliceAgentDouble::TableSliceAgentDouble(Table* table,
                              TableSliceAgent* masterAgent,
                              Operable* requiredColIdx,
                              int requiredColIdxInt) :
            _table(table),
            _masterAgent(masterAgent),
            _requiredColIdx(requiredColIdx),
            _requiredColIdxInt(requiredColIdxInt){
        assert(_table != nullptr);
        assert(_masterAgent != nullptr);

    }

    void TableSliceAgentDouble::doStaticGlobAsm(int idx, Operable& opr){

        Slot createdSlot(_table->getMeta()(idx, idx+1));
        createdSlot.addHwFieldMeta({&opr, nullptr});
        _masterAgent->doGlobAsm(createdSlot);

    }

    void TableSliceAgentDouble::doStaticGlobAsm(int idx, ull srcVal){
        Operable& mySrcOpr = getMatchAssignOperable(srcVal, _table->getMaxCellWidth());
        doStaticGlobAsm(idx, mySrcOpr);
    }



    Operable& TableSliceAgentDouble::v(){

        if (isStaticColRead()){
            return _masterAgent->v()(_requiredColIdxInt);
        }
        ///// case dynamic get value both row and column
        return _masterAgent->v()[*_requiredColIdx].v();
    }

    TableSliceAgentDouble&
        TableSliceAgentDouble::operator <<= (Operable& rhsOpr){

        if (isStaticColRead()){
            ///// let the master agent do it for you
            doStaticGlobAsm(_requiredColIdxInt, rhsOpr);
        }else{
            _table->doGlobAsm(rhsOpr, *_requiredColIdx,
                              *_requiredColIdx, ASM_DIRECT, _masterAgent->isOneHotEncMode());
        }

        return *this;
    }

    TableSliceAgentDouble&
        TableSliceAgentDouble::operator <<= (ull rhsVal){

        if (isStaticColRead()){
            ///// let the master agent do it for you
            doStaticGlobAsm(_requiredColIdxInt, rhsVal);
        }else{
            _table->doGlobAsm(rhsVal, *_requiredColIdx,
                              *_requiredColIdx, ASM_DIRECT, _masterAgent->isOneHotEncMode());
        }

        return *this;

    }


}