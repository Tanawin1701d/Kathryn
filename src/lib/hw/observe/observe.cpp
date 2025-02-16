//
// Created by tanawin on 16/2/2025.
//

#include "observe.h"


namespace kathryn{

    Operable& observe(const std::vector<Operable*>& signals, Operable* resetEvent){

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

    Operable& observe(const std::vector<Operable*>& signals, const std::vector<Operable*>& resetEvents){
        Operable* totalRst = nullptr;
        if (!resetEvents.empty()){
            totalRst = resetEvents[0];
            for (int i = 1; i < resetEvents.size(); i++){
                totalRst = &((*totalRst) | (*resetEvents[i]));
            }
        }
        return observe(signals, totalRst);
    }



}