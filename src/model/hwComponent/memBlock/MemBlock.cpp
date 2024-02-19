//
// Created by tanawin on 18/2/2567.
//

#include "MemBlock.h"

namespace kathryn{


    MemBlock::MemBlock(ull depth, ull width):
            Identifiable(TYPE_MEM_BLOCK),
            HwCompControllerItf(false),
            RtlSimulatable(nullptr),
            ModelDebuggable(),
            DEPTH_SIZE(depth),
            WIDTH_SIZE(width)
    {}

    MemBlockEleHolder& MemBlock::operator[](const Operable& indexer) {
        auto* memBlockEleHolder = new MemBlockEleHolder(this, &indexer);
        memBlockAgent.push_back(memBlockEleHolder);
        return *memBlockEleHolder;
    }

    MemBlockEleHolder& MemBlock::operator[](const int idx) {
        auto* memBlockEleHolder = new MemBlockEleHolder(this, idx);
        memBlockAgent.push_back(memBlockEleHolder);
        return *memBlockEleHolder;
    }



}
