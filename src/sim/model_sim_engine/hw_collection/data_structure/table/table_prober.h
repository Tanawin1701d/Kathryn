//
// Created by tanawin on 11/10/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLEPROBER_H
#define SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLEPROBER_H
#include "vector"

#include "sim/model_sim_engine/flow_block/flow_block_prober.h"
#include "sim/model_sim_engine/hw_collection/data_structure/slot/slot_prober.h"
#include "sim/sim_res_writer/sim_res_writer.h"

namespace kathryn{



    struct Table;
    struct TableSimProbe{
        Table* table = nullptr;
        std::vector<SlotSimProbe> row_sim_probes;


        void                       init(Table* table);

        SlotSimInfo64              get_cur_values(int row_idx);
        FieldSimInfo64             get_cur_values(int row_idx, std::string var_name);
        std::vector<SlotSimInfo64> get_cur_values();

        std::vector<SlotSimInfo64> detect_row_change(); //// if there is any change in row bring it entire row to display
        void                       apply_change();



    };


}

#endif //SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLEPROBER_H
