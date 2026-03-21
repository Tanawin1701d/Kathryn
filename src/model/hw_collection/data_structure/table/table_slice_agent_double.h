//
// Created by tanawin on 16/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENTDOUBLE_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENTDOUBLE_H

#include "model/hw_collection/data_structure/slot/reg_slot.h"
#include "model/hw_collection/data_structure/slot/wire_slot.h"

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
                              TableSliceAgent* master_agent,
                              Operable* required_col_idx,
                              int required_col_idx_int);

        void do_static_glob_asm(int idx, Operable& opr1);

        void do_static_glob_asm(int idx, ull val);

        bool is_static_col_read(){ return _requiredColIdx == nullptr; }

        Operable& v();

        TableSliceAgentDouble& operator <<= (Operable& rhs_opr);

        TableSliceAgentDouble& operator <<= (ull rhs_val);
    };
} // namespace kathryn

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENTDOUBLE_H
