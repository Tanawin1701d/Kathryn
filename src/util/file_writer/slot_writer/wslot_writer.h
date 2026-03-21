//
// Created by tanawin on 21/12/25.
//

#ifndef UTIL_FILEWRITER_SLOTWRITER_WSLOTWRITER_H
#define UTIL_FILEWRITER_SLOTWRITER_WSLOTWRITER_H
#include "queue"

#include "slot_writer_base.h"


namespace kathryn{


    class WSlotWriter: public SlotWriterBase{
        const int WINDOW_SIZE;
    protected:
        std::queue<ROW> _qRows;


    public:
        WSlotWriter(const std::vector<std::string>& slot_names,
                    int column_width,
                    const std::string& file_name,
                    int window_size = 10);
        WSlotWriter(const std::vector<std::string>& slot_names,
                    const std::vector<int>& col_widths,
                    const std::string& file_name,
                    int window_size = 10);

        void renew(const std::string& file_name) override;

        void add_slot_val (int slot_idx, const std::string& value) override;
        void add_slot_vals(int slot_idx, const std::vector<std::string>& values) override;

        //////// use when we want to save file with this current window situation
        void finalize_last_window();

        void conclude_each_cycle() override;

        void init() override;

    };

}

#endif //UTIL_FILEWRITER_SLOTWRITER_WSLOTWRITER_H