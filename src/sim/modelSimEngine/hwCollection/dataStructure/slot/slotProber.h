//
// Created by tanawin on 11/10/25.
//

#ifndef SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTPROBER_H
#define SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTPROBER_H
#include <vector>

#include "sim/simResWriter/simResWriter.h"

namespace kathryn{

    struct FieldSimInfo64{
        std::string name;
        ull         prevValue;
        ull         curValue;
    };

    struct SlotSimInfo64{
        int rowIdx = -1;
        std::vector<FieldSimInfo64> fieldSimInfos;

        void append(const FieldSimInfo64& info){
            fieldSimInfos.push_back(info);
        }

        FieldSimInfo64& operator[](int idx){ return fieldSimInfos[idx];}

    };

    class Slot;
    struct SlotSimProbe{

        Slot* slot = nullptr;
        std::vector<ull> prevValues; ///// the index of vector is also the index of the slot

        void init(Slot* slot);

        FieldSimInfo64 getCurSimInfo(int col);

        SlotSimInfo64  getCurValues();
        FieldSimInfo64 getCurValue(std::string varName);

        /////// we can get only the change system
        bool isThereChange();
        std::vector<FieldSimInfo64> detectChange();
        void applyChange();




    };

}

#endif //SRC_SIM_MODELSIMENGINE_HWCOLLECTION_DATASTRUCTURE_SLOT_SLOTPROBER_H
