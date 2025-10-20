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

    std::string SlotWriter::ROW::getResultRow(std::vector<int> slotWidths) {
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

    /***
     *
     * SLOTWRITER
     *
     * */

    SlotWriter::SlotWriter(std::vector<std::string> slotNames, int columnWidth, std::string fileName):
        FileWriterBase(fileName),
        SLOTSIZE(slotNames.size()),
        _slotWidth(slotNames.size(), columnWidth),
        _slotNames(slotNames)
        {
        /***initialize value and head of table*/
        SlotWriter::init();


    }

    SlotWriter::SlotWriter(std::vector<std::string> slotNames, std::vector<int> colWidths, std::string fileName):
        FileWriterBase(fileName),
        SLOTSIZE(slotNames.size()),
        _slotWidth(colWidths),
        _slotNames(slotNames)
    {
        /***initialize value and head of table*/
        assert(SLOTSIZE == colWidths.size());
        SlotWriter::init();

    }



    void SlotWriter::addSlotVal(int slotIdx, std::string value) {
        auto& curRow = *_rows.rbegin();
        curRow.addSlotVal(slotIdx, std::move(value));
    }

    void SlotWriter::iterateCycle() {

        /***write the data*/
        auto& curRow = *_rows.rbegin();
        std::string getData = curRow.getResultRow(_slotWidth);
        addData(getData);
        std::string breakVal;
        breakVal += "+";
        for (int colWidth: _slotWidth){
            for (int i = 0; i < (colWidth + 1); i++){
                breakVal += "-";
            }
        }

        breakVal += "\n";
        addData(breakVal);

        /***clear vector and create new row*/
            /** we buffer row for further upgrade but for now we neglect that*/
        _rows.clear();
        _rows.emplace_back(SLOTSIZE);
    }

    void SlotWriter::init(){
        assert(SLOTSIZE > 0);
        _rows.clear();
        _rows.emplace_back(SLOTSIZE);
        for (int i = 0; i < SLOTSIZE; i++){
            addSlotVal(i, _slotNames[i]);
        }
        iterateCycle();
    }



}