//
// Created by tanawin on 21/12/25.
//

#ifndef UTIL_FILEWRITER_SLOTWRITER_SLOTWRITERBASE_H
#define UTIL_FILEWRITER_SLOTWRITER_SLOTWRITERBASE_H

#include "cassert"
#include "vector"
#include "util/file_writer/file_writer_base.h"



namespace kathryn{


    class SlotWriterBase: public FileWriterBase{

    protected:

        const int SLOTSIZE    = 1;
        struct ROW{
            /** each slot might have several lines*/
            typedef  std::vector<std::string> SLOTDATA;
            std::vector<SLOTDATA> col; /////// array of slot

            explicit ROW(int sl_size){ col.resize(sl_size);}

            int         find_max_slot_data_line(); //// find max line in all slot
            void        add_slot_val  (int slot_idx, const std::string& value);
            std::string get_result_row(std::vector<int> slot_widths);

        };

        std::vector<std::string> _slotNames;
        std::vector<int>         _slotWidth;


    public:

        SlotWriterBase(const std::vector<std::string>& slot_names, int column_width,const std::string& file_name);
        SlotWriterBase(const std::vector<std::string>& slot_names, std::vector<int> col_widths, const std::string& file_name);


        virtual void  add_slot_val (int slot_idx, const std::string& value) = 0;
        virtual void  add_slot_vals(int slot_idx, const std::vector<std::string>& values) = 0;
        virtual void conclude_each_cycle() = 0;

        std::string gen_slot_break_val();


    };

}

#endif //UTIL_FILEWRITER_SLOTWRITER_SLOTWRITERBASE_H