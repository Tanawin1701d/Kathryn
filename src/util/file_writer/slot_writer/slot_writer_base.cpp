//
// Created by tanawin on 21/12/25.
//

#include "slot_writer_base.h"

#include "utility"

namespace kathryn{


    /***
     *
     * ROW
     *
     * */

    int SlotWriterBase::ROW::find_max_slot_data_line() {
        int max_ln = 0;
        for(auto& slot: col){
            max_ln = std::max(max_ln, (int)slot.size());
        }
        return max_ln;
    }

    void SlotWriterBase::ROW::add_slot_val(int slot_idx, const std::string& value) {
        assert(slot_idx < col.size());
        assert(value.find('\n') == std::string::npos);
        col[slot_idx].push_back(value);
    }

    std::string SlotWriterBase::ROW::get_result_row(std::vector<int> slot_widths) {
        int amt_line = find_max_slot_data_line();

        std::string result;
        for (int ln = 0; ln < amt_line; ln++){
            result += "|";
            for (int sl_idx = 0; sl_idx < col.size(); sl_idx++){
                int slot_width = slot_widths[sl_idx];
                /**check there is no line for print just nothing*/
                if (ln < col[sl_idx].size()){
                    /** before padding*/
                    int before_space_amt = (slot_width - ((int)col[sl_idx][ln].size()) + 1)/2;
                    for (int pd = 0; pd < before_space_amt; pd++){ result += ' ';}
                    /** in processing data*/
                    result += col[sl_idx][ln];
                    /** after padding*/
                    int back_space_amt = (slot_width - (int)col[sl_idx][ln].size())/2;
                    for (int pd = 0; pd < back_space_amt; pd++){
                        result += ' ';
                    }
                }else{
                    for (int pd = 0; pd < slot_width; pd++){ result += ' ';}
                }
                result += "|";
            }
            result += '\n';
        }

        return result;
    }


    SlotWriterBase::SlotWriterBase(const std::vector<std::string>& slot_names,
                                   int column_width,
                                   const std::string& file_name):
        FileWriterBase(file_name),
        SLOTSIZE(slot_names.size()),
        _slotWidth(slot_names.size(), column_width),
        _slotNames(slot_names){}

    SlotWriterBase::SlotWriterBase(const std::vector<std::string>& slot_names,
                                   std::vector<int> col_widths, const std::string& file_name):
        FileWriterBase(file_name),
        SLOTSIZE(slot_names.size()),
        _slotWidth(std::move(col_widths)),
        _slotNames(slot_names){}


    std::string SlotWriterBase::gen_slot_break_val() {
        std::string break_val;
        break_val += "+";
        for (int col_width: _slotWidth){
            for (int i = 0; i < (col_width + 1); i++){
                break_val += "-";
            }
        }
        break_val += "\n";
        return break_val;
    }



}