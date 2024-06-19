//
// Created by tanawin on 18/2/2567.
//

#include "MemBlock.h"
#include "model/controller/controller.h"

namespace kathryn{


    MemBlock::MemBlock(ull depth, int width):
            Identifiable(TYPE_MEM_BLOCK),
            HwCompControllerItf(false),
            ModelDebuggable(),
            DEPTH_SIZE(depth),
            WIDTH_SIZE(width),
            memSimEngine(new MemSimEngine(this))
    {
        assert(width > 0);
        com_init();
    }

    MemBlock::~MemBlock(){
        for (auto mba: memBlockAgents){
            delete mba;
        }
    }

    void MemBlock::com_init(){
        ctrl->on_memBlk_init(this);
    }

    MemBlockEleHolder& MemBlock::operator[](const Operable& indexer) {
        auto* memBlockEleHolder = new MemBlockEleHolder(this, &indexer);
        memBlockAgents.push_back(memBlockEleHolder);
        return *memBlockEleHolder;
    }

    MemBlockEleHolder& MemBlock::operator[](const int idx) {
        auto* memBlockEleHolder = new MemBlockEleHolder(this, idx);
        memBlockAgents.push_back(memBlockEleHolder);
        return *memBlockEleHolder;
    }

    ValRepBase MemBlock::at(ull idx){
        mfAssert(getAssignMode() == AM_SIM, "can't retrive memblock while"
                                            " at " +std::to_string(idx) + " loading");
        mfAssert(idx < getDepthSize(), "can't retrive memblock while"
                                            " at " +std::to_string(idx) + " exceed index value");
        return (*getSimEngine()->getProxyRep())[idx];
    }


}
