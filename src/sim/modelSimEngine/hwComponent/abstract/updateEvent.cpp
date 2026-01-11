//
// Created by tanawin on 27/11/25.
//

#include "sim/modelSimEngine/hwComponent/abstract/updateEvent.h"
#include "model/hwComponent/abstract/updateEvent.h"
#include "sim/modelSimEngine/hwComponent/abstract/logicSimEngine.h"

#include "genHelper.h"


namespace kathryn{

    /**
     * update event base
     *
     */

    UpdateEventBaseSimEngine::~UpdateEventBaseSimEngine(){
        for (UpdateEventBaseSimEngine* simEngine: subEngine){
            delete simEngine;
        }
    }

    /**
     * update event basic
     *
     */

    void UpdateEventBasicSimEngine::createSimOp(CbBaseCxx& cb,
                                        LogicSimEngine& lse,
                                        const std::string& auxAssStr){

        std::string assStr = lse.genAssignAEqB(master->_desSlice, lse.isTempReq(), master->_value);
        cb.addSt(assStr);
        if (!auxAssStr.empty()){
            cb.addSt(auxAssStr); //// make it more beutiful
        }
    }

    /**
     * update grp event
     *
     ***/
    void UpdateEventGrpSimEngine::createSimOp(CbBaseCxx& cb,
                                     LogicSimEngine& lse,
                                     const std::string& auxAssStr){

        for(UpdateEventBase* ueb: master->subStmts){

            UpdateEventBaseSimEngine* simEngine = ueb->createSimEvent();
            subEngine.push_back(simEngine);
            simEngine->createSimOp(cb, lse, auxAssStr);
        }

    }

    /**
     * update cond event
     *
     */

    void UpdateEventCondSimEngine::createSimOp(CbBaseCxx& cb,
                                      LogicSimEngine& logicSimEngine,
                                      const std::string& auxAssStr){

        CbIfCxx* firstIfStmt = nullptr;
        assert(master->conditions.size() == master->subStmts.size());
        for (int i = 0; i < master->conditions.size(); ++i){
            Operable* condition  = master->conditions[i];
            UpdateEventBase* ueb = master->subStmts[i];
            std::string condStr  = "true";
            if (condition != nullptr){
                condStr = getSlicedSrcOprFromOpr(condition).toString();
            }
            UpdateEventBaseSimEngine* simEngine = ueb->createSimEvent();

            ///// creating the block
            CbIfCxx* curWriteBlock = nullptr;

            if (firstIfStmt == nullptr){
                firstIfStmt = &cb.addIf(condStr);
                curWriteBlock = firstIfStmt;
            }else{
                curWriteBlock = &firstIfStmt->addElif(condStr);

            }
            simEngine->createSimOp(*curWriteBlock, logicSimEngine, auxAssStr);
            //simEngine->createSimOp(*firstIfStmt, logicSimEngine, auxAssStr);
            subEngine.push_back(simEngine);

        }
    }

    /**
     * update state event
     */

    void UpdateEventSwitchSimEngine::createSimOp(CbBaseCxx& cb,
                                      LogicSimEngine& logicSimEngine,
                                      const std::string& auxAssStr){

        //////// get switch identifier
        Operable& stateIden  = master->stateIden;
        std::string identStr = getSlicedSrcOprFromOpr(&stateIden).toString();
        //////// build add ident to cxx block
        CbSwitchCxx* switchCxx = &cb.addSwitch(identStr);

        for (int idx = 0; idx < master->getMatchNum(); idx++){
            UpdateEventBase*          ueb       = master->getSubStmts(idx);
            int                       matchIdx  = master->getSubStmtMatchIdxs(idx);
            UpdateEventBaseSimEngine* simEngine = ueb->createSimEvent();

            CbBaseCxx* matchWorkBlock = &switchCxx->addCase(matchIdx);
            simEngine->createSimOp(*matchWorkBlock, logicSimEngine, auxAssStr);

            subEngine.push_back(simEngine);
        }



    }
}
