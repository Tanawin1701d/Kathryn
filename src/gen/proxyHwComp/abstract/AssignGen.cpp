//
// Created by tanawin on 22/6/2024.
//

#include "AssignGen.h"

#include "gen/proxyHwComp/module/moduleGen.h"

namespace kathryn{


    AssignGenBase::~AssignGenBase(){
        translatedUpdatePool.clean();
    }


    void AssignGenBase::routeDep(){

        assert(_asb != nullptr);
        _asb->sortUpEventByPriority();
        /** copy the translatedUpdatePool*/
        translatedUpdatePool = _asb->getUpdateMeta().clone();
        /** try to reroute the update Event*/
        for(UpdateEventBase* ueb: translatedUpdatePool.events){
            UEBaseGenEngine* genEngine = ueb->createGenEngine();
            genEngine->reroute(_mdGenMaster);
            delete genEngine;
        }
    }

    std::pair<Verilog_SEN_TYPE, std::string>
    AssignGenBase::getClockSenInfo(UpdateEventBase* ueb){

        assert(ueb != nullptr);
        switch (ueb->_clkMode){
            case CM_POSEDGE: return std::make_pair(VLST_POSEDGE, "clk");
            case CM_NEGEDGE: return std::make_pair(VLST_NEGEDGE, "clk");
            default: return std::make_pair(VLST_ALWAYS, "*");
        }

    }



    std::string AssignGenBase::assignOpWithSoleCondition(){

        std::string retStr;
        CbBaseVerilog cb;

        for (UpdateEventBase* ueb: translatedUpdatePool.events){
            auto [verSenType, senSig] = getClockSenInfo(ueb);

            CbAlwaysVerilog cbAw = cb.addAlways(verSenType, senSig);
            UEBaseGenEngine* genEngine = ueb->createGenEngine();
            genEngine->genAss(cbAw, this);
            delete genEngine;
        }

        retStr = cb.toString(4);
        return retStr;

    }



    std::string AssignGenBase::assignOpBase(){
        return assignOpWithSoleCondition();

    }

    std::string AssignGenBase::assignmentLine(Slice desSlice, Operable* srcUpdateValue, bool isDelayedAsm){
        assert(srcUpdateValue != nullptr);
        std::string asmOpr = isDelayedAsm ? " <= " : " = ";
        return getOpr(desSlice) + asmOpr + getOprStrFromOprAndShinkMsb(srcUpdateValue, desSlice.getSize()) + ";";
    }
}
