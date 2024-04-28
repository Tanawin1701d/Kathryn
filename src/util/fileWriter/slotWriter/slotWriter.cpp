//
// Created by tanawin on 25/4/2567.
//

#include <cassert>
#include "slotWriter.h"


namespace kathryn{



    /***
     *
     * ROW
     *
     * */

    int SlotWriter::ROW::findMaxSlotDataLine() {
        int maxLn = 0;
        for(auto& slot: col){
            maxLn = std::max(maxLn, (int)slot.size());
        }
        return maxLn;
    }

    void SlotWriter::ROW::addSlotVal(int slotIdx, const std::string& value) {
        assert(slotIdx < col.size());
        assert(value.find('\n') == std::string::npos);
        col[slotIdx].push_back(value);
    }

    std::string SlotWriter::ROW::getResultRow(int slotWidth) {
        int amtLine = findMaxSlotDataLine();

        std::string result;
        for (int ln = 0; ln < amtLine; ln++){
            result += "|";
            for (int slIdx = 0; slIdx < col.size(); slIdx++){
                /**check there is no line for print just nothing*/
                if (ln < col[slIdx].size()){
                    /** before padding*/
                    for (int pd = 0; pd < (slotWidth - col[slIdx][ln].size() + 1)/2; pd++){ result += ' ';}
                    /** in processing data*/
                    result += col[slIdx][ln];
                    /** after padding*/
                    for (int pd = 0; pd < (slotWidth - col[slIdx][ln].size())/2; pd++){ result += ' ';}
                }else{
                    for (int pd = 0; pd < slotWidth; pd++){ result += ' ';}
                }
                result += "|";
            }
            result += '\n';
        }

        return result;
    }

    /***
     *
     * SLOTWRITER
     *
     * */

    SlotWriter::SlotWriter(std::vector<std::string> slotNames, int columnWidth, std::string fileName):
        FileWriterBase(fileName),
        SLOTSIZE(slotNames.size()),
        COLUMNWIDTH(columnWidth),
        _slotNames(slotNames)
        {
        /***initialize value and head of table*/
        assert(SLOTSIZE > 0);
        _rows.emplace_back(SLOTSIZE);
        for (int i = 0; i < SLOTSIZE; i++){
            addSlotVal(i, _slotNames[i]);
        }
        iterateCycle();

    }

    void SlotWriter::addSlotVal(int slotIdx, std::string value) {
        auto& curRow = *_rows.rbegin();
        curRow.addSlotVal(slotIdx, std::move(value));
    }

    void SlotWriter::iterateCycle() {

        /***write the data*/
        auto& curRow = *_rows.rbegin();
        addData(curRow.getResultRow(COLUMNWIDTH));
        std::string breakVal;
        for (int i = 0; i < (SLOTSIZE * (COLUMNWIDTH + 1)); i++){
            breakVal += "-";
        }
        breakVal += "\n";
        addData(breakVal);

        /***clear vector and create new row*/
            /** we buffer row for further upgrade but for now we neglect that*/
        _rows.clear();
        _rows.emplace_back(SLOTSIZE);
    }


}