//
// Created by tanawin on 11/10/25.
//

#include "table_prober.h"
#include "model/hw_collection/data_structure/table/table.h"

namespace kathryn{

    void TableSimProbe::init(Table* target_table){
        table = target_table;
        assert(table != nullptr);
        for (int idx = 0; idx < table->get_num_row(); idx++){
            row_sim_probes.emplace_back(SlotSimProbe());
            row_sim_probes.rbegin()->init(&table->get_ref_row(idx));
        }
    }


    SlotSimInfo64 TableSimProbe::get_cur_values(int row_idx){
        mf_assert(table->is_valid_idx(row_idx), "get cur sim info at " + std::to_string(row_idx) + " out of range");
        return row_sim_probes[row_idx].get_cur_values();
    }

    FieldSimInfo64 TableSimProbe::get_cur_values(int row_idx, std::string var_name){
        return row_sim_probes[row_idx].get_cur_value(var_name);
    }

    std::vector<SlotSimInfo64> TableSimProbe::get_cur_values(){
        int row_idx = 0;
        std::vector<SlotSimInfo64> results;
        for (auto& probe : row_sim_probes){
            results.push_back(probe.get_cur_values());
            results.back().row_idx = row_idx;
            row_idx++;
        }
        return results;
    }

    std::vector<SlotSimInfo64> TableSimProbe::detect_row_change(){
        int row_idx = 0;
        std::vector<SlotSimInfo64> change_row;
        for (auto& probe : row_sim_probes){
            if (probe.is_there_change()){
                change_row.push_back(probe.get_cur_values());
                change_row.back().row_idx = row_idx;
            }
            row_idx++;
        }
        return change_row;
    }

    void TableSimProbe::apply_change(){
        for (auto& probe : row_sim_probes){
            probe.apply_change();
        }
    }
}
