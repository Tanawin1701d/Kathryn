//
// Created by tanawin on 21/12/25.
//

#include "wslotWriter.h"


namespace kathryn{

    WSlotWriter::WSlotWriter(const std::vector<std::string>& slotNames,
                           int columnWidth,
                           const std::string& fileName,
                           int windowSize):
    SlotWriterBase(slotNames, columnWidth, fileName),
    WINDOW_SIZE(windowSize){
        assert(WINDOW_SIZE > 0);
        WSlotWriter::init();
    }

    WSlotWriter::WSlotWriter(const std::vector<std::string>& slotNames,
                           const std::vector<int>& colWidths,
                           const std::string& fileName,
                           int windowSize):
    SlotWriterBase(slotNames, colWidths, fileName),
    WINDOW_SIZE(windowSize){
        /***initialize value and head of table*/
        assert(WINDOW_SIZE > 0);
        assert(SLOTSIZE == colWidths.size());
        WSlotWriter::init();
    }

    void WSlotWriter::addSlotVal(int slotIdx,
                                 const std::string& value){

        assert(!_qRows.empty());
        _qRows.back().addSlotVal(slotIdx, value);



    }
    void WSlotWriter::addSlotVals(int slotIdx,
                                  const std::vector<std::string>& values){
        for (auto& val: values){
            addSlotVal(slotIdx, val);
        }
    }

    void WSlotWriter::iterateCycle(){

        if (_qRows.size() == WINDOW_SIZE){
            _qRows.pop();
        }
        ///// create new slot for next iteration
        _qRows.emplace(SLOTSIZE);
    }

    void WSlotWriter::finalizeLastWindow(){

        while (!_qRows.empty()){
            std::string getData = _qRows.front().getResultRow(_slotWidth);
            _qRows.pop();
            addData(getData);
            std::string breakVal = genSlotBreakVal();
            addData(breakVal);
        }
        _qRows.emplace(SLOTSIZE);

    }

    void WSlotWriter::init(){
        assert(SLOTSIZE > 0);
        _qRows.emplace(SLOTSIZE);
    }

}