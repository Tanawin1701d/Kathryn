//
// Created by tanawin on 18/2/2567.
//

#ifndef KATHRYN_MEMBLOCK_H
#define KATHRYN_MEMBLOCK_H

#include <map>
#include "model/hwComponent/abstract/identifiable.h"
#include "model/controller/conInterf/controllerItf.h"
#include "model/simIntf/modelSimInterface.h"
#include "model/debugger/modelDebugger.h"
#include "model/hwComponent/abstract/operable.h"
#include "MemBlockAgent.h"

namespace kathryn{

    class MemBlock: public Identifiable,
                    public HwCompControllerItf,
                    public RtlSimulatable,
                    public ModelDebuggable{
    private:
        const ull DEPTH_SIZE = 0;
        const int WIDTH_SIZE = 0;

        std::vector<MemBlockEleHolder*> memBlockAgent;


    public:
        explicit MemBlock(ull depth, ull width);

        MemBlockEleHolder& operator[] (const Operable& indexer);

        MemBlockEleHolder& operator[] (const int idx);

        int getWidthSize() const {assert(WIDTH_SIZE != 0); return WIDTH_SIZE;}




    };

}

#endif //KATHRYN_MEMBLOCK_H
