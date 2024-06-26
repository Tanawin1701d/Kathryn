//
// Created by tanawin on 22/6/2024.
//

#ifndef ASSIGNGEN_H
#define ASSIGNGEN_H

#include "logicGenBase.h"

namespace kathryn{

    class AssignGenBase: public LogicGenBase{
    protected:

        std::vector<UpdateEvent*> translatedUpdateEvent;
    public:
        explicit AssignGenBase(ModuleGen*    mdGenMaster,
                              logicLocalCef cerf,
                              Assignable*   asb,
                              Identifiable* ident
        ):LogicGenBase(mdGenMaster, std::move(cerf), asb, ident){}

        ~AssignGenBase() override;

        void routeDep() override;

        std::string assignOpBase(bool isClockSen);

        virtual std::string assignmentLine(Slice desSlice, Operable* srcUpdateValue);

        void addDirectUpdateEvent(UpdateEvent* updateEvent) override{
            assert(updateEvent != nullptr);
            translatedUpdateEvent.push_back(updateEvent);
        }



    };


}

#endif //ASSIGNGEN_H
