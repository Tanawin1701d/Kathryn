//
// Created by tanawin on 16/9/25.
//

#ifndef SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENT_H
#define SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENT_H

#include "table_slice_agent_double.h"
#include "model/hw_collection/data_structure/slot/wire_slot.h"



namespace kathryn{
    class Table;


    class TableSliceAgent{
    protected:
        bool _oneHotEncMode = false;
        Table* _table = nullptr;
        Operable& _required_idx;

    public:
        TableSliceAgent(Table* table, Operable& required_idx, bool one_hot_enc_mode) :
            _table(table),
            _required_idx(required_idx),
            _oneHotEncMode(one_hot_enc_mode){
        }

        WireSlot v();

        bool is_one_hot_enc_mode() const{ return _oneHotEncMode;}

        void do_glob_asm(Slot& src_slot);
        TableSliceAgent& operator <<= (Slot& rhs);

        TableSliceAgentDouble operator () (int idx);
        TableSliceAgentDouble operator () (const std::string& name);

        TableSliceAgentDouble operator [] (Operable& required_idx);


    };
}

#endif //SRC_MODEL_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLESLICEAGENT_H
