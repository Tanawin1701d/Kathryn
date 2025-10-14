//
// Created by tanawin on 11/10/25.
//

#include "tableProber.h"
#include "model/hwCollection/dataStructure/table/table.h"

namespace kathryn{

    void TableSimProbe::init(Table* targetTable){
        table = targetTable;
        assert(table != nullptr);
        for (int idx = 0; idx < table->getNumRow(); idx++){
            rowSimProbes.emplace_back(SlotSimProbe());
            rowSimProbes.rbegin()->init(&table->getRefRow(idx));
        }
    }


    SlotSimInfo64 TableSimProbe::getCurValues(int rowIdx){
        mfAssert(table->isValidIdx(rowIdx), "get cur sim info at " + std::to_string(rowIdx) + " out of range");
        return rowSimProbes[rowIdx].getCurValues();
    }

    FieldSimInfo64 TableSimProbe::getCurValues(int rowIdx, std::string varName){
        return rowSimProbes[rowIdx].getCurValue(varName);
    }

    std::vector<SlotSimInfo64> TableSimProbe::getCurValues(){
        int rowIdx = 0;
        std::vector<SlotSimInfo64> results;
        for (auto& probe : rowSimProbes){
            results.push_back(probe.getCurValues());
            results.back().rowIdx = rowIdx;
            rowIdx++;
        }
        return results;
    }

    std::vector<SlotSimInfo64> TableSimProbe::detectRowChange(){
        int rowIdx = 0;
        std::vector<SlotSimInfo64> changeRow;
        for (auto& probe : rowSimProbes){
            if (probe.isThereChange()){
                changeRow.push_back(probe.getCurValues());
                changeRow.back().rowIdx = rowIdx;
            }
            rowIdx++;
        }
        return changeRow;
    }

    void TableSimProbe::applyChange(){
        for (auto& probe : rowSimProbes){
            probe.applyChange();
        }
    }
}
