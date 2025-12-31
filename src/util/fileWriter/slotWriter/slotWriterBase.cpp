//
// Created by tanawin on 21/12/25.
//

#include "slotWriterBase.h"

#include <utility>

namespace kathryn{


    /***
     *
     * ROW
     *
     * */

    int SlotWriterBase::ROW::findMaxSlotDataLine() {
        int maxLn = 0;
        for(auto& slot: col){
            maxLn = std::max(maxLn, (int)slot.size());
        }
        return maxLn;
    }

    void SlotWriterBase::ROW::addSlotVal(int slotIdx, const std::string& value) {
        assert(slotIdx < col.size());
        assert(value.find('\n') == std::string::npos);
        col[slotIdx].push_back(value);
    }

    std::string SlotWriterBase::ROW::getResultRow(std::vector<int> slotWidths) {
        int amtLine = findMaxSlotDataLine();

        std::string result;
        for (int ln = 0; ln < amtLine; ln++){
            result += "|";
            for (int slIdx = 0; slIdx < col.size(); slIdx++){
                int slotWidth = slotWidths[slIdx];
                /**check there is no line for print just nothing*/
                if (ln < col[slIdx].size()){
                    /** before padding*/
                    int beforeSpaceAmt = (slotWidth - ((int)col[slIdx][ln].size()) + 1)/2;
                    for (int pd = 0; pd < beforeSpaceAmt; pd++){ result += ' ';}
                    /** in processing data*/
                    result += col[slIdx][ln];
                    /** after padding*/
                    int backSpaceAmt = (slotWidth - (int)col[slIdx][ln].size())/2;
                    for (int pd = 0; pd < backSpaceAmt; pd++){
                        result += ' ';
                    }
                }else{
                    for (int pd = 0; pd < slotWidth; pd++){ result += ' ';}
                }
                result += "|";
            }
            result += '\n';
        }

        return result;
    }


    SlotWriterBase::SlotWriterBase(const std::vector<std::string>& slotNames,
                                   int columnWidth,
                                   const std::string& fileName):
        FileWriterBase(fileName),
        SLOTSIZE(slotNames.size()),
        _slotWidth(slotNames.size(), columnWidth),
        _slotNames(slotNames){}

    SlotWriterBase::SlotWriterBase(const std::vector<std::string>& slotNames,
                                   std::vector<int> colWidths, const std::string& fileName):
        FileWriterBase(fileName),
        SLOTSIZE(slotNames.size()),
        _slotWidth(std::move(colWidths)),
        _slotNames(slotNames){}


    std::string SlotWriterBase::genSlotBreakVal() {
        std::string breakVal;
        breakVal += "+";
        for (int colWidth: _slotWidth){
            for (int i = 0; i < (colWidth + 1); i++){
                breakVal += "-";
            }
        }
        breakVal += "\n";
        return breakVal;
    }



}