//
// Created by tanawin on 25/4/2567.
//

#ifndef KATHRYN_SLOTWRITER_H
#define KATHRYN_SLOTWRITER_H

#include<vector>
#include "util/fileWriter/fileWriterBase.h"

namespace kathryn{

    class SlotWriter: public FileWriterBase{
        const int SLOTSIZE    = 1;
        const int COLUMNWIDTH = 1;

        struct ROW{
            /** each slot might have several line*/
            typedef  std::vector<std::string> SLOTDATA;
            std::vector<SLOTDATA> col; /////// array of slot

            explicit ROW(int slSize){ col.resize(slSize);}

            int         findMaxSlotDataLine(); //// find max line in all slot
            void        addSlotVal  (int slotIdx, const std::string& value);
            std::string getResultRow(int slotWidth);


        };

        std::vector<std::string> _slotNames;
        std::vector<ROW> _rows;


    public:
        explicit SlotWriter(std::vector<std::string> slotNames, int columnWidth,std::string fileName);

        void addSlotVal(int slotIdx, std::string value);

        void iterateCycle();


    };


}

#endif //KATHRYN_SLOTWRITER_H
