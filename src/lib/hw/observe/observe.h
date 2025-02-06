//
// Created by tanawin on 5/2/2025.
//

#ifndef LIB_HW_TRACKKER_TRACKER_H
#define LIB_HW_TRACKKER_TRACKER_H

#include "model/controller/controller.h"

namespace kathryn{

    Operable& observe(const std::vector<Operable*>& signals, Operable* resetEvent = nullptr){

        mfAssert(signals.size() <= 64, "for now observer is not support > 64 signal for stability of system");

        Val&  fullUp   = mOprVal ("upFullState"  , signals.size(), genBiConValRep(true , signals.size()));
        Val&  fullDown = mOprVal ("downFullState", signals.size(), genBiConValRep(false, signals.size()));
        Val&  upState  = mOprVal("upState"       , 1  , 1);
        Wire& obWire   = mOprWire("observeWire"  , signals.size()); ///// it will show the result immediately
        Reg&  obReg    = mOprReg ("observeReg"   , signals.size()); ///// it will save the old result

        int idx = 0;
        for(Operable* obsSignal: signals){
            /////// action on wire signal
            auto* wireTestEvent = new UpdateEvent({
                obsSignal,
                nullptr,
                &upState,
                Slice({idx, idx+1}),
                DEFAULT_UE_PRI_INTERNAL_MAX
            });
            obWire.addUpdateMeta(wireTestEvent);
            /////// action on reg signal
            auto* regTestEvent = new UpdateEvent({
                obsSignal,
                nullptr,
                &upState,
                Slice({idx, idx+1}),
                DEFAULT_UE_PRI_INTERNAL_MAX
            });
            obReg.addUpdateMeta(regTestEvent);
            idx++;
        }
        //////// add reset event
        Operable& finSignal = (obWire | obReg) == fullUp;
        auto* pushDownEvent = new UpdateEvent({
                (resetEvent == nullptr) ? (&finSignal) : &(finSignal | (*resetEvent)),
                nullptr,
                &fullDown,
                Slice({0, static_cast<int>(signals.size())}),
                DEFAULT_UE_PRI_INTERNAL_MAX
        });
        obReg.addUpdateMeta(pushDownEvent);

        return finSignal;
    }


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
