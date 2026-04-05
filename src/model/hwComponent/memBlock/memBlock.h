//
// Created by tanawin on 18/2/2567.
//

#ifndef KATHRYN_MEMBLOCK_H
#define KATHRYN_MEMBLOCK_H

#include "map"
#include "sim/modelSimEngine/hwComponent/memBlk/memSim.h"
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"
#include "model/debugger/modelDebugger.h"
#include "model/hwComponent/abstract/operable.h"
#include "util/numberic/pmath.h"
#include "gen/proxyHwComp/memBlock/memGen.h"

#include "memBlockAgent.h"


namespace kathryn{

    class MemBlock: public Identifiable,
                    public HwCompControllerItf,
                    public MemSimEngineInterface,
                    public LogicGenInterface,
                    public ModelDebuggable{
    private:
        const ull DEPTH_SIZE = 0;
        const int WIDTH_SIZE = 0;

        std::vector<MemBlockEleHolder*> memBlockAgents;

        MemSimEngine* memSimEngine = nullptr;
        LogicGenBase* _genEngine = nullptr;

    public:
        explicit MemBlock(ull depth, int width);
        ~MemBlock();

        void comInit() override;
        void comFinal() override{};

        void initEnvForMemHolder(MemBlockEleHolder* memEleHolder);

        MemBlockEleHolder& operator[] (const Operable& indexer);
        MemBlockEleHolder& operator[] (const int idx);

        int getWidthSize() const {assert(WIDTH_SIZE != 0); return WIDTH_SIZE;}
        ull getDepthSize() const {assert(DEPTH_SIZE != 0); return DEPTH_SIZE;}
        auto& getMemBlockAgents(){return memBlockAgents;}

        ValRepBase at(ull idx);

        /** override debugger*/
        std::string getMdIdentVal() override{
            return getIdentDebugValue();
        }

        MemSimEngine* getSimEnginePtr()                 override{return memSimEngine;}
        void          markSv(const std::string& str) override{memSimEngine->markSv(str);}

        void          createLogicGen() override;
        LogicGenBase* getLogicGenPtr() override{return _genEngine;}

    };

}

#endif //KATHRYN_MEMBLOCK_H
