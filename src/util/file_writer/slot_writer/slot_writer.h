//
// Created by tanawin on 25/4/2567.
//

#ifndef KATHRYN_SLOTWRITER_H
#define KATHRYN_SLOTWRITER_H

#include "slot_writer_base.h"

namespace kathryn{

    class SlotWriter: public SlotWriterBase{

    protected:

        std::vector<ROW> _rows;

    public:
        SlotWriter(const std::vector<std::string>& slot_names, int column_width,const std::string& file_name);
        SlotWriter(std::vector<std::string> slot_names, const std::vector<int>& col_widths, std::string file_name);

        void add_slot_val (int slot_idx, const std::string& value) override;
        void add_slot_vals(int slot_idx, const std::vector<std::string>& values) override;
        void conclude_each_cycle() override;

        void init() override;
    };


}

#endif //KATHRYN_SLOTWRITER_H
