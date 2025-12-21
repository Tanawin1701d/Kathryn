//
// Created by tanawin on 25/4/2567.
//

#include <cassert>
#include <utility>
#include "slotWriter.h"


namespace kathryn{


    /***
     *
     * SLOTWRITER
     *
     * */

    SlotWriter::SlotWriter(const std::vector<std::string>& slotNames,
                           int columnWidth,
                           const std::string& fileName):
    SlotWriterBase(slotNames, columnWidth, fileName){
        SlotWriter::init();
    }

    SlotWriter::SlotWriter(std::vector<std::string> slotNames,
                           const std::vector<int>& colWidths,
                           std::string fileName):
    SlotWriterBase(std::move(slotNames), colWidths, std::move(fileName)){
        /***initialize value and head of table*/
        assert(SLOTSIZE == colWidths.size());
        SlotWriter::init();
    }

    void SlotWriter::addSlotVal(int slotIdx, const std::string& value) {
        auto& curRow = *_rows.rbegin();
        curRow.addSlotVal(slotIdx, value);
    }

    void SlotWriter::addSlotVals(int slotIdx, const std::vector<std::string>& values) {
        for (auto& val: values){
            addSlotVal(slotIdx, val);
        }
    }

    void SlotWriter::iterateCycle() {

        /***write the data*/
        auto& curRow = *_rows.rbegin();
        std::string getData = curRow.getResultRow(_slotWidth);
        addData(getData);
        std::string breakVal = genSlotBreakVal();
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