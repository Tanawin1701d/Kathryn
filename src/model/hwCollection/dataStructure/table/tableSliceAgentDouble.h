//
// Created by tanawin on 16/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENTDOUBLE_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENTDOUBLE_H

#include "model/hwCollection/dataStructure/slot/regSlot.h"
#include "model/hwCollection/dataStructure/slot/wireSlot.h"

namespace kathryn{
    class Table;
    class TableSliceAgent;
    class Operable;

    class TableSliceAgentDouble{
    protected:
        Table*           _table             = nullptr;
        TableSliceAgent* _masterAgent       = nullptr;
        Operable*        _requiredColIdx    = nullptr;
        int              _requiredColIdxInt;

    public:
        TableSliceAgentDouble(Table* table,
                              TableSliceAgent* masterAgent,
                              Operable* requiredColIdx,
                              int requiredColIdxInt);

        void doStaticGlobAsm(int idx, Operable& opr);

        bool isStaticColRead(){ return _requiredColIdx == nullptr; }

        Operable& v();

        TableSliceAgentDouble& operator <<= (Operable& rhsOpr);
    };
} // namespace kathryn

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENTDOUBLE_H
