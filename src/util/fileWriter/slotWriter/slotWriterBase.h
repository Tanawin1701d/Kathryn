//
// Created by tanawin on 21/12/25.
//

#ifndef UTIL_FILEWRITER_SLOTWRITER_SLOTWRITERBASE_H
#define UTIL_FILEWRITER_SLOTWRITER_SLOTWRITERBASE_H

#include <cassert>
#include<vector>
#include "util/fileWriter/fileWriterBase.h"



namespace kathryn{


    class SlotWriterBase: public FileWriterBase{

    protected:

        const int SLOTSIZE    = 1;
        struct ROW{
            /** each slot might have several lines*/
            typedef  std::vector<std::string> SLOTDATA;
            std::vector<SLOTDATA> col; /////// array of slot

            explicit ROW(int slSize){ col.resize(slSize);}

            int         findMaxSlotDataLine(); //// find max line in all slot
            void        addSlotVal  (int slotIdx, const std::string& value);
            std::string getResultRow(std::vector<int> slotWidths);

        };

        std::vector<std::string> _slotNames;
        std::vector<int>         _slotWidth;


    public:

        SlotWriterBase(const std::vector<std::string>& slotNames, int columnWidth,const std::string& fileName);
        SlotWriterBase(const std::vector<std::string>& slotNames, std::vector<int> colWidths, const std::string& fileName);


        virtual void  addSlotVal (int slotIdx, const std::string& value) = 0;
        virtual void  addSlotVals(int slotIdx, const std::vector<std::string>& values) = 0;
        virtual void concludeEachCycle() = 0;

        std::string genSlotBreakVal();


    };

}

#endif //UTIL_FILEWRITER_SLOTWRITER_SLOTWRITERBASE_H