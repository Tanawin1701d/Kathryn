//
// Created by tanawin on 16/9/25.
//

#include "table_slice_agent_double.h"

#include "table.h"
#include "table_slice_agent.h"

namespace kathryn{

    TableSliceAgentDouble::TableSliceAgentDouble(Table* table,
                              TableSliceAgent* master_agent,
                              Operable* required_col_idx,
                              int required_col_idx_int) :
            _table(table),
            _masterAgent(master_agent),
            _requiredColIdx(required_col_idx),
            _requiredColIdxInt(required_col_idx_int){
        assert(_table != nullptr);
        assert(_masterAgent != nullptr);

    }

    void TableSliceAgentDouble::do_static_glob_asm(int idx, Operable& opr1){

        Slot created_slot(_table->get_meta()(idx, idx+1));
        created_slot.add_hw_field_meta({&opr1, nullptr});
        _masterAgent->do_glob_asm(created_slot);

    }

    void TableSliceAgentDouble::do_static_glob_asm(int idx, ull src_val){
        Operable& my_src_opr = get_match_assign_operable(src_val, _table->get_max_cell_width());
        do_static_glob_asm(idx, my_src_opr);
    }



    Operable& TableSliceAgentDouble::v(){

        if (is_static_col_read()){
            return _masterAgent->v()(_requiredColIdxInt);
        }
        ///// case dynamic get value both row and column
        return _masterAgent->v()[*_requiredColIdx].v();
    }

    TableSliceAgentDouble&
        TableSliceAgentDouble::operator <<= (Operable& rhs_opr){

        if (is_static_col_read()){
            ///// let the master agent do it for you
            do_static_glob_asm(_requiredColIdxInt, rhs_opr);
        }else{
            _table->do_glob_asm(rhs_opr, *_requiredColIdx,
                              *_requiredColIdx, ASM_DIRECT, _masterAgent->is_one_hot_enc_mode());
        }

        return *this;
    }

    TableSliceAgentDouble&
        TableSliceAgentDouble::operator <<= (ull rhs_val){

        if (is_static_col_read()){
            ///// let the master agent do it for you
            do_static_glob_asm(_requiredColIdxInt, rhs_val);
        }else{
            _table->do_glob_asm(rhs_val, *_requiredColIdx,
                              *_requiredColIdx, ASM_DIRECT, _masterAgent->is_one_hot_enc_mode());
        }

        return *this;

    }


}