//
// Created by tanawin on 11/10/25.
//

#include "slot_prober.h"
#include "model/hw_collection/data_structure/slot/slot.h"

namespace kathryn{

    void SlotSimProbe::init(Slot* input_slot){

        this->slot = input_slot;
        mf_assert(slot != nullptr, "slot is null");
        prev_values.resize(slot->get_num_field_ptr(),0);
    }


    FieldSimInfo64 SlotSimProbe::get_cur_sim_info(int col){
        mf_assert(slot->is_valid_idx(col), "get cur sim info at " +
            std::to_string(col) + " out of range");

        FieldMeta   field       = slot->get_meta().get_copy_field(col);
        HwFieldMeta hw_meta      = slot->hw_field_at(col);
        ull         prev_value = prev_values[col];
        ull         cur_value    = (ull)(*hw_meta._opr);
        return {field._name, prev_value, cur_value};

    }

    /**
     *    get cur simulation value
     ***/
    SlotSimInfo64 SlotSimProbe::get_cur_values(){

        SlotSimInfo64 result;
        ///////// loop to retrive all meta
        for (int idx = 0; idx < slot->get_num_field_ptr(); idx++){
            result.append(get_cur_sim_info(idx));
        }
        return result;

    }

    FieldSimInfo64 SlotSimProbe::get_cur_value(std::string var_name){

        int idx = slot->get_meta().get_idx_ptr(var_name);
        return get_cur_sim_info(idx);

    }

    /**
     * change value detection
     */

    bool SlotSimProbe::is_there_change(){
        SlotSimInfo64 cur_vals = get_cur_values();

        for (int idx = 0; idx < slot->get_num_field_ptr(); idx++){
            if (prev_values[idx] != cur_vals[idx].cur_value){
                return true;
            }
        }
        return false;

    }


    std::vector<FieldSimInfo64> SlotSimProbe::detect_change(){

        SlotSimInfo64 cur_vals = get_cur_values();
        std::vector<FieldSimInfo64> result;

        for (int idx = 0; idx < slot->get_num_field_ptr(); idx++){
            FieldSimInfo64 cur_val = cur_vals[idx];
            if (cur_val.prev_value != cur_val.cur_value){
                result.push_back(cur_val);
            }
        }
        return result;
    }

    void SlotSimProbe::apply_change(){
        SlotSimInfo64 cur_vals = get_cur_values();

        for (int idx = 0; idx < slot->get_num_field_ptr(); idx++){
            prev_values[idx] = cur_vals[idx].cur_value;
        }

    }


}