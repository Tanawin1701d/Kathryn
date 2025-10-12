//
// Created by tanawin on 11/10/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLEPROBER_H
#define SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLEPROBER_H
#include <vector>

#include "sim/modelSimEngine/flowBlock/flowBlockProber.h"
#include "sim/modelSimEngine/hwCollection/dataStructure/slot/slotProber.h"
#include "sim/simResWriter/simResWriter.h"

namespace kathryn{



    struct Table;
    struct TableSimProbe{
        Table* table = nullptr;
        std::vector<SlotSimProbe> rowSimProbes;

        TableSimProbe(Table* table);

        SlotSimInfo64              getCurValues(int rowIdx);
        FieldSimInfo64             getCurValues(int rowIdx, std::string varName);
        std::vector<SlotSimInfo64> getCurValues();

        std::vector<SlotSimInfo64> detectRowChange(); //// if there is any change in row bring it entire row to display
        void                       applyChange();



    };


}

#endif //SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_TABLE_TABLEPROBER_H
