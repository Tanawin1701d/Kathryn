//
// Created by tanawin on 16/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENT_H

#include "tableSliceAgentDouble.h"
#include "model/hwCollection/dataStructure/slot/wireSlot.h"



namespace kathryn{
    class Table;


    class TableSliceAgent{
    protected:
        Table* _table = nullptr;
        Operable& _requiredIdx;

    public:
        TableSliceAgent(Table* table, Operable& requiredIdx) :
            _table(table),
            _requiredIdx(requiredIdx){
        }

        WireSlot v();

        void doGlobAsm(Slot& srcSlot);
        TableSliceAgent& operator <<= (Slot& rhs);

        TableSliceAgentDouble operator () (int idx);
        TableSliceAgentDouble operator () (const std::string& name);

        TableSliceAgentDouble operator [] (Operable& requiredIdx);


    };
}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENT_H
