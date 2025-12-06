//
// Created by tanawin on 22/6/2024.
//

#ifndef ASSIGNGEN_H
#define ASSIGNGEN_H

#include "logicGenBase.h"

namespace kathryn{

    class AssignGenBase: public LogicGenBase{
    protected:

        UpdatePool translatedUpdatePool;


    public:
        explicit AssignGenBase(ModuleGen*    mdGenMaster,
                              Assignable*   asb,
                              Identifiable* ident
        ):LogicGenBase(mdGenMaster, asb, ident){}

        ~AssignGenBase() override;

        void routeDep() override;

        std::pair<Verilog_SEN_TYPE, std::string> getClockSenInfo();

        /**
         *  assign system
         */
        //std::string assignOpWithChainCondition(bool isClockSen);
        std::string assignOpWithSoleCondition();
        std::string assignOpBase();

        virtual std::string assignmentLine(Slice desSlice,
                                           Operable* srcUpdateValue,
                                           bool isDelayedAsm);


        void addDirectUpdateEvent(UpdateEventBase* ueb) override{
            assert(ueb != nullptr);
            translatedUpdatePool.addUpdateEvent(ueb);
        }

    };


}

#endif //ASSIGNGEN_H
