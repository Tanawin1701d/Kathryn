//
// Created by tanawin on 9/12/25.
//

#ifndef MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H
#define MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H
#include "model/flowBlock/abstract/nodes/asmNode.h"
#include "model/hwComponent/abstract/operable.h"
#include "model/hwComponent/wire/wire.h"

namespace kathryn{


    inline Operable& mux(Operable& sel, Operable& x0, Operable& x1){

        assert(sel.getOperableSlice().getSize() == 1);
        assert(x0.getOperableSlice().getSize() == x1.getOperableSlice().getSize());
        int destinatedWidth = x0.getOperableSlice().getSize();
        assert(destinatedWidth > 0);

        Wire& result = mOprWire("mux2", destinatedWidth);

        AssignMeta* sel0 =
        result.generateAssignMeta(x0, result.getOperableSlice(), ASM_DIRECT, CM_CLK_FREE);
        AssignMeta* sel1 =
        result.generateAssignMeta(x1, result.getOperableSlice(), ASM_DIRECT, CM_CLK_FREE);

        AsmNode* assNode = new AsmNode({sel0, sel1});
        assNode->addSpecificPreCondition(&(sel == 1), 0); ///// for x0
        assNode->addSpecificPreCondition(&(sel == 0), 1); ///// for x1
        assNode->dryAssign();
        delete assNode;

        return result;


    }

    inline Operable& mux(Operable& sel, const std::vector<Operable*>& srcs){

        int indexSize   = sel.getOperableSlice().getSize();
        int capableSize = 1 << indexSize;
        assert(indexSize >= 1);
        mfAssert(capableSize == srcs.size(), "the input amount should be 2^(sel)");

        //// initialize queue
        std::queue<Operable*> muxQueue;
        for (int idx = 0; idx < capableSize; idx++){
            assert(srcs[idx] != nullptr);
            muxQueue.push(srcs[idx]);
        }

        //// generate mux
        int bitIdx = 0;
        while (muxQueue.size() > 1){
            int size = muxQueue.size();
            for (int piece = 0; piece < size; piece+=2){
                Operable* first   = muxQueue.front(); muxQueue.pop();
                Operable* second  = muxQueue.front(); muxQueue.pop();
                Operable& nextOpr = mux(*sel.doSlice({bitIdx, bitIdx+1}), *second, *first);
                muxQueue.push(&nextOpr);
            }
            bitIdx +=1;
        }

        assert(muxQueue.size() == 1);
        return *muxQueue.front();
    }



}

#endif //MODEL_HWCOLLECTION_DATASTRUCTURE_MUX_MUX_H