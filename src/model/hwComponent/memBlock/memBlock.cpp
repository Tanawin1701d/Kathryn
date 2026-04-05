//
// Created by tanawin on 18/2/2567.
//

#include "memBlock.h"

#include "gen/proxyHwComp/memBlock/memGen.h"
#include "model/controller/controller.h"
#include "sim/modelSimEngine/hwComponent/memBlk/memSim.h"

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
        comInit();
    }

    MemBlock::~MemBlock(){
        for (auto mba: memBlockAgents){
            delete mba;
        }
    }

    void MemBlock::comInit(){
        ctrl->on_memBlk_init(this);
    }

    void MemBlock::initEnvForMemHolder(MemBlockEleHolder* memEleHolder){
        assert(memEleHolder != nullptr);
        assert(getParentPtr() != nullptr);
        memEleHolder->setParent(getParentPtr());
        memEleHolder->buildInheritName();
        memBlockAgents.pushBack(memEleHolder);
    }


    MemBlockEleHolder& MemBlock::operator[](const Operable& indexer) {
        auto* memBlockEleHolder = new MemBlockEleHolder(this, &indexer);
        initEnvForMemHolder(memBlockEleHolder);
        return *memBlockEleHolder;
    }

    MemBlockEleHolder& MemBlock::operator[](const int idx) {
        auto* memBlockEleHolder = new MemBlockEleHolder(this, idx);
        initEnvForMemHolder(memBlockEleHolder);
        return *memBlockEleHolder;
    }

    ValRepBase MemBlock::at(ull idx){
        mfAssert(getAssignMode() == AM_SIM, "can't retrive memblock while"
                                            " at " +std::toString(idx) + " loading");
        mfAssert(idx < getDepthSize(), "can't retrive memblock while"
                                            " at " +std::toString(idx) + " exceed index value");
        return (getSimEnginePtr()->getProxyRep())[idx];
    }

    void MemBlock::createLogicGen(){
        _genEngine = new MemGen(
            _parent->getModuleGenPtr(),
            this
        );
    }

}
