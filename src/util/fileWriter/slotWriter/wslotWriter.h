//
// Created by tanawin on 21/12/25.
//

#ifndef UTIL_FILEWRITER_SLOTWRITER_WSLOTWRITER_H
#define UTIL_FILEWRITER_SLOTWRITER_WSLOTWRITER_H
#include <queue>

#include "slotWriterBase.h"


namespace kathryn{


    class WSlotWriter: public SlotWriterBase{
        const int WINDOW_SIZE;
    protected:
        std::queue<ROW> _qRows;


    public:
        WSlotWriter(const std::vector<std::string>& slotNames,
                    int columnWidth,
                    const std::string& fileName,
                    int windowSize = 10);
        WSlotWriter(const std::vector<std::string>& slotNames,
                    const std::vector<int>& colWidths,
                    const std::string& fileName,
                    int windowSize = 10);

        void addSlotVal (int slotIdx, const std::string& value) override;
        void addSlotVals(int slotIdx, const std::vector<std::string>& values) override;

        //////// use when we want to save file with this current window situation
        void finalizeLastWindow();

        void iterateCycle();

        void init() override;

    };

}

#endif //UTIL_FILEWRITER_SLOTWRITER_WSLOTWRITER_H