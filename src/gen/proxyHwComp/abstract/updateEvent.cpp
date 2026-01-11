//
// Created by tanawin on 27/11/25.
//

#include "gen/proxyHwComp/abstract/updateEvent.h"
#include "gen/proxyHwComp/module/moduleGen.h"


namespace kathryn{
    /**
     * gen base
     */

    UEBaseGenEngine::~UEBaseGenEngine(){
        for (UEBaseGenEngine* genEngine: subEngine){
            delete genEngine;
        }
    }

    Operable* UEBaseGenEngine::rerouteBase(Operable* srcOpr, ModuleGen* mdGen){
        if (srcOpr == nullptr){
            return nullptr;
        }
        return mdGen->routeSrcOprToThisModule(srcOpr);
    }

    void UEBaseGenEngine::rerouteAndReplace(Operable*& srcOpr, ModuleGen* mdGen){
        srcOpr = rerouteBase(srcOpr, mdGen);
    }

    /**
     * basic
     */
    bool UEBasicGenEngine::validateAssignSensivity() const{
        return ((master->_clkMode == CM_POSEDGE)  ||
               (master->_clkMode == CM_NEGEDGE)  ||
               (master->_clkMode == CM_CLK_FREE)
               );
    }


    void UEBasicGenEngine::reroute(ModuleGen* mdGen){
        rerouteAndReplace(master->_value, mdGen);
    }

    void UEBasicGenEngine::genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen){
        assert(validateAssignSensivity());
        bool isClockSen = (master->_clkMode == CM_POSEDGE)  || (master->_clkMode == CM_NEGEDGE);
        std::string assStr = assignGen->assignmentLine(
            master->_desSlice, master->_value, isClockSen
        );
        cbVer.addSt(assStr);
    }

    void UEBasicGenEngine::genBasicConnect(CbBaseVerilog& cbVer, AssignGenBase* assignGen){

        std::string assStr = "assign " + assignGen->getOpr() +
                             " = "     +  assignGen->getOprStrFromOpr(master->_value);
        cbVer.addSt(assStr);
    }

    /**
     * grp
     */
    void UEGrpGenEngine::reroute(ModuleGen* mdGen){
        for (UpdateEventBase* ueb: master->subStmts){
            UEBaseGenEngine* genEngine = ueb->createGenEngine();
            genEngine->reroute(mdGen);
            subEngine.push_back(genEngine);
        }
    }

    void UEGrpGenEngine::genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen){

        for(UpdateEventBase* ueb: master->subStmts){
            UEBaseGenEngine* genEngine = ueb->createGenEngine();
            subEngine.push_back(genEngine);
            genEngine->genAss(cbVer, assignGen);
        }

    }

    /**
     * cond
     */
    void UECondGenEngine::reroute(ModuleGen* mdGen){
        for (int idx = 0; idx < master->conditions.size(); idx++){
            rerouteAndReplace(master->conditions[idx], mdGen);
        }
        for (UpdateEventBase* ueb: master->subStmts){
            UEBaseGenEngine* genEngine = ueb->createGenEngine();
            subEngine.push_back(genEngine);
            genEngine->reroute(mdGen);
        }
    }

    void UECondGenEngine::genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen){

        CbIfVerilog* firstIfStmt = nullptr;
        assert(master->conditions.size() == master->subStmts.size());
        for (int i = 0; i < master->conditions.size(); ++i){
            Operable* condition = master->conditions[i];
            UpdateEventBase* ueb = master->subStmts[i];
            std::string condStr = "1'b1";
            if (condition != nullptr){
                condStr = assignGen->getOprStrFromOpr(condition);
            }
            UEBaseGenEngine* genEngine = ueb->createGenEngine();

            ///// creating the block
            CbIfVerilog* curBlock = nullptr;

            if (firstIfStmt == nullptr){
                firstIfStmt = &cbVer.addIf(condStr);
                curBlock    = firstIfStmt;
            }else{
                curBlock    = &firstIfStmt->addElif(condStr);
            }
            genEngine->genAss(*curBlock, assignGen);
            subEngine.push_back(genEngine);

        }

    }

    void UECondGenEngine::genBasicConnect(CbBaseVerilog& cbVer, AssignGenBase* assignGen){
        assert(master->conditions.size() == 1);
        assert(master->subStmts.size() == 1);
        assert(master->conditions[0] == nullptr);
        auto* genEngine = master->subStmts[0]->createGenEngine();
        genEngine->genBasicConnect(cbVer, assignGen);
        subEngine.push_back(genEngine);

    }

    /**
     *
     * switch
     */
    void UESwitchGenEngine::reroute(ModuleGen* mdGen){
        Operable* ident = &master->getStateIdent();
        rerouteAndReplace(ident, mdGen);
        for (UpdateEventBase* ueb: master->subStmts){
            UEBaseGenEngine* genEngine = ueb->createGenEngine();
            genEngine->reroute(mdGen);
            subEngine.push_back(genEngine);
        }
    }

    void UESwitchGenEngine::genAss(CbBaseVerilog& cbVer, AssignGenBase* assignGen){

        std::string stateIdent = assignGen->getOprStrFromOpr(&master->getStateIdent());
         CbSwitchVerilog* cbVerSwitch = &cbVer.addSwitch(stateIdent);

        for (int idx = 0; idx < master->getMatchNum(); idx++){
            UEBaseGenEngine* genEngine = master->getSubStmts(idx)->createGenEngine();
            int              matchIdx    = master->getSubStmtMatchIdxs(idx);
            CbBaseVerilog* caseBlock = &cbVerSwitch->addCase(matchIdx);
            genEngine->genAss(*caseBlock, assignGen);
            subEngine.push_back(genEngine);
        }


    }

}