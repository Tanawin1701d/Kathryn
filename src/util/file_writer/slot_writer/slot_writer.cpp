//
// Created by tanawin on 25/4/2567.
//

#include "cassert"
#include "utility"
#include "slot_writer.h"


namespace kathryn{


    /***
     *
     * SLOTWRITER
     *
     * */

    SlotWriter::SlotWriter(const std::vector<std::string>& slot_names,
                           int column_width,
                           const std::string& file_name):
    SlotWriterBase(slot_names, column_width, file_name){
        SlotWriter::init();
    }

    SlotWriter::SlotWriter(std::vector<std::string> slot_names,
                           const std::vector<int>& col_widths,
                           std::string file_name):
    SlotWriterBase(std::move(slot_names), col_widths, std::move(file_name)){
        /***initialize value and head of table*/
        assert(SLOTSIZE == col_widths.size());
        SlotWriter::init();
    }

    void SlotWriter::add_slot_val(int slot_idx, const std::string& value) {
        auto& cur_row = *_rows.rbegin();
        cur_row.add_slot_val(slot_idx, value);
    }

    void SlotWriter::add_slot_vals(int slot_idx, const std::vector<std::string>& values) {
        for (auto& val: values){
            add_slot_val(slot_idx, val);
        }
    }

    void SlotWriter::conclude_each_cycle() {

        /***write the data*/
        auto& cur_row = *_rows.rbegin();
        std::string get_data = cur_row.get_result_row(_slotWidth);
        add_data(get_data);
        std::string break_val = gen_slot_break_val();
        add_data(break_val);
        /***clear vector and create new row*/
            /** we buffer row for further upgrade but for now we neglect that*/
        _rows.clear();
        _rows.emplace_back(SLOTSIZE);
    }

    void SlotWriter::init(){
        assert(SLOTSIZE > 0);
        _rows.clear();
        _rows.emplace_back(SLOTSIZE);
        for (int i = 0; i < SLOTSIZE; i++){
            add_slot_val(i, _slotNames[i]);
        }
        conclude_each_cycle();
    }



}