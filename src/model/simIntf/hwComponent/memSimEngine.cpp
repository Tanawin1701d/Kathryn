//
// Created by tanawin on 19/2/2567.
//

#include "model/simIntf/hwComponent/memSimEngine.h"


#include<iostream>


namespace kathryn{


    MemSimEngine::MemSimEngine(ull depthSize, int widthSize):
    memBlk(nullptr),
    DEPTH_SIZE(depthSize),
    WIDTH_SIZE(widthSize)
    {
         /** we will not allocate memory
          * fo memory block right now
          * we will allocate in prepare sim session
          * */
    }

    void
    MemSimEngine::prepareSim(){
        memBlk =  new ValRep[DEPTH_SIZE];
        for (int i = 0; i < DEPTH_SIZE; i++){
            memBlk[i] = ValRep(WIDTH_SIZE);
        }

    }

    ValRep&
    MemSimEngine::getThisCycleValRep(ull idx){
        assert(memBlk != nullptr);
        assert(idx < DEPTH_SIZE);
        return memBlk[idx];
    }

    ValRep&
    MemSimEngine::getNextCycleVapRepSrc(kathryn::ull idx) {
        assert(idx < DEPTH_SIZE);
        if (pendingWrite.find(idx) == pendingWrite.end()){
            pendingWrite[idx] = memBlk[idx];
        }
        return pendingWrite[idx];
    }
}