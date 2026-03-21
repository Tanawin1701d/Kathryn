//
// Created by tanawin on 11/10/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTPROBER_H
#define SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTPROBER_H
#include "vector"

#include "sim/sim_res_writer/sim_res_writer.h"

namespace kathryn{

    struct FieldSimInfo64{
        std::string name;
        ull         prev_value;
        ull         cur_value;
    };

    struct SlotSimInfo64{
        int row_idx = -1;
        std::vector<FieldSimInfo64> field_sim_infos;

        void append(const FieldSimInfo64& info){
            field_sim_infos.push_back(info);
        }

        FieldSimInfo64& operator[](int idx){ return field_sim_infos[idx];}

    };

    class Slot;
    struct SlotSimProbe{

        Slot* slot = nullptr;
        std::vector<ull> prev_values; ///// the index of vector is also the index of the slot

        void init(Slot* slot);

        FieldSimInfo64 get_cur_sim_info(int col);

        SlotSimInfo64  get_cur_values();
        FieldSimInfo64 get_cur_value(std::string var_name);

        /////// we can get only the change system
        bool is_there_change();
        std::vector<FieldSimInfo64> detect_change();
        void apply_change();




    };

}

#endif //SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTPROBER_H
