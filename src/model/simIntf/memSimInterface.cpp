//
// Created by tanawin on 19/2/2567.
//

#include "memSimInterface.h"


#include<iostream>


namespace kathryn{


    MemSimulatable::MemSimulatable(ull depthSize, int widthSize):
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
    MemSimulatable::prepareSim(){
        memBlk =  new ValRep[DEPTH_SIZE];
        for (int i = 0; i < DEPTH_SIZE; i++){
            memBlk[i] = ValRep(WIDTH_SIZE);
        }

    }

    ValRep&
    MemSimulatable::getThisCycleValRep(ull idx){
        assert(memBlk != nullptr);
        assert(idx < DEPTH_SIZE);
        return memBlk[idx];
    }
}