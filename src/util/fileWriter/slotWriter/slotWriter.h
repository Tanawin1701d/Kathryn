//
// Created by tanawin on 25/4/2567.
//

#ifndef KATHRYN_SLOTWRITER_H
#define KATHRYN_SLOTWRITER_H

#include "slotWriterBase.h"

namespace kathryn{

    class SlotWriter: public SlotWriterBase{

    protected:

        std::vector<ROW> _rows;

    public:
        SlotWriter(const std::vector<std::string>& slotNames, int columnWidth,const std::string& fileName);
        SlotWriter(std::vector<std::string> slotNames, const std::vector<int>& colWidths, std::string fileName);

        void addSlotVal (int slotIdx, const std::string& value);
        void addSlotVals(int slotIdx, const std::vector<std::string>& values);
        void iterateCycle();

        void init() override;
    };


}

#endif //KATHRYN_SLOTWRITER_H
