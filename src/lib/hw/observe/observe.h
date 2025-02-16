//
// Created by tanawin on 5/2/2025.
//

#ifndef LIB_HW_TRACKKER_TRACKER_H
#define LIB_HW_TRACKKER_TRACKER_H

#include "model/controller/controller.h"

namespace kathryn{

    Operable& observe(const std::vector<Operable*>& signals, Operable* resetEvent = nullptr);
    Operable& observe(const std::vector<Operable*>& signals, const std::vector<Operable*>& resetEvents);


    ///////// input is operable&
    template<typename... Args>
    Operable& obs(Args... sigs){
        std::vector<Operable*> signals = {&sigs...};
        return observe(signals, nullptr);
    }
    template<typename... Args>
    Operable& obsWRst(Operable& rst, Args... sigs){
        std::vector<Operable*> signals = {&sigs...};
        return observe(signals, &rst);
    }


    template<typename... Args>
    void blk_obsAndWait(Args... sigs){
        Operable& summarizeSig = obs(sigs...);
        scWait(summarizeSig);
    }

    template<typename... Args>
    void blk_obsWRstAndWait(Operable& rst, Args... sigs){
        Operable& summarizeSig = obsWRst(rst, sigs...);
        scWait(summarizeSig);
    }



}

#endif //LIB_HW_TRACKKER_TRACKER_H
