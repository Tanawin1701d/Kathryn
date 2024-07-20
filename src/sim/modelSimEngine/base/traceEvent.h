//
// Created by tanawin on 20/7/2024.
//

#ifndef TRACEEVENT_H
#define TRACEEVENT_H
#include "model/hwComponent/abstract/operable.h"
#include "sim/modelSimEngine/hwComponent/abstract/genHelper.h"

namespace kathryn{


    struct TraceEvent{
        Operable* _condOpr = nullptr;
        void (*_callback)();

        TraceEvent(Operable& opr, void (*callback)()):
        _condOpr(&opr),
        _callback(callback){
            assert(opr.getOperableSlice().getSize() == 1);
        }

        [[nodiscard]] std::string getCondStr() const{
            return getSlicedSrcOprFromOpr(_condOpr);
        }

    };





}

#endif //TRACEEVENT_H
