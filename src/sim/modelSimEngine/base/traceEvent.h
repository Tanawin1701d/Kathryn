//
// Created by tanawin on 20/7/2024.
//

#ifndef TRACEEVENT_H
#define TRACEEVENT_H
#include <sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h>

#include "model/hwComponent/abstract/operable.h"
#include "sim/modelSimEngine/hwComponent/abstract/genHelper.h"

namespace kathryn{


    struct TraceEvent{
        Operable* _condOpr = nullptr;
        std::function<void(void)> _callback;

        TraceEvent(Operable& opr, std::function<void(void)> callback):
        _condOpr(&opr),
        _callback(callback){
            assert(opr.getOperableSlice().getSize() == 1);
             LogicSimEngine* lse = opr.getLogicSimEngineFromOpr();
            assert(lse != nullptr);
            lse->reqGlobDec();
        }

        [[nodiscard]] std::string getCondStr() const{
            return getSlicedSrcOprFromOpr(_condOpr).toString();
        }

        void execCallBack() const{
            _callback();
        }

    };





}

#endif //TRACEEVENT_H
