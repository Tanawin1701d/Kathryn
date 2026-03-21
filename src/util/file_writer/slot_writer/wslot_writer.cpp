//
// Created by tanawin on 21/12/25.
//

#include "wslot_writer.h"


namespace kathryn{

    WSlotWriter::WSlotWriter(const std::vector<std::string>& slot_names,
                           int column_width,
                           const std::string& file_name,
                           int window_size):
    SlotWriterBase(slot_names, column_width, file_name),
    WINDOW_SIZE(window_size){
        assert(WINDOW_SIZE > 0);
        WSlotWriter::init();
    }

    WSlotWriter::WSlotWriter(const std::vector<std::string>& slot_names,
                           const std::vector<int>& col_widths,
                           const std::string& file_name,
                           int window_size):
    SlotWriterBase(slot_names, col_widths, file_name),
    WINDOW_SIZE(window_size){
        /***initialize value and head of table*/
        assert(WINDOW_SIZE > 0);
        assert(SLOTSIZE == col_widths.size());
        WSlotWriter::init();
    }

    void WSlotWriter::renew(const std::string& file_name){
        finalize_last_window();
        SlotWriterBase::renew(file_name);
    }

    void WSlotWriter::add_slot_val(int slot_idx,
                                 const std::string& value){

        assert(!_qRows.empty());
        _qRows.back().add_slot_val(slot_idx, value);



    }
    void WSlotWriter::add_slot_vals(int slot_idx,
                                  const std::vector<std::string>& values){
        for (auto& val: values){
            add_slot_val(slot_idx, val);
        }
    }

    void WSlotWriter::conclude_each_cycle(){

        if (_qRows.size() == WINDOW_SIZE){
            _qRows.pop();
        }
        ///// create new slot for next iteration
        _qRows.emplace(SLOTSIZE);
    }

    void WSlotWriter::finalize_last_window(){

        while (!_qRows.empty()){
            std::string get_data = _qRows.front().get_result_row(_slotWidth);
            _qRows.pop();
            add_data(get_data);
            std::string break_val = gen_slot_break_val();
            add_data(break_val);
        }
        _qRows.emplace(SLOTSIZE);

    }

    void WSlotWriter::init(){
        assert(SLOTSIZE > 0);
        _qRows.emplace(SLOTSIZE);
    }

}