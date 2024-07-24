//
// Created by tanawin on 31/5/2024.
//

#include "logicSimEngine.h"

#include "genHelper.h"
#include "util/numberic/numConvert.h"
#include "sim/controller/simController.h"


namespace kathryn{
    LogicSimEngine::LogicSimEngine(Assignable* asb, Identifiable* ident,
                                   VCD_SIG_TYPE sigType, bool isTempReq,
                                   ull initVal):
        _asb(asb),
        _ident(ident),
        _vcdSigType(sigType),
        _isTempReq(isTempReq),
        _initVal(initVal){
        assert(asb != nullptr);
        assert(ident != nullptr);
        //assert(_vcdSigType != VCD_SIG_TYPE::VST_DUMMY);
        ///////// fill asb  to system to support topology sort
    }

    void LogicSimEngine::createOpWithSoleCondition(CbBaseCxx& cb,
                                                   const std::string& auxAssStr){
        for (UpdateEvent* updateEvent : _asb->getUpdateMeta()){
            ////
            ///check integrity
            ///
            assert(updateEvent->srcUpdateValue->getOperableSlice().getSize() >=
                updateEvent->desUpdateSlice.getSize());
            std::string condStr;
            std::string assStr;
            bool isConOccur = false;
            ////  gather activate condition
            if (updateEvent->srcUpdateState != nullptr){
                condStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateState);
                isConOccur = true;
            }
            if (updateEvent->srcUpdateCondition != nullptr){
                if (isConOccur){
                    condStr += " && ";
                }
                condStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateCondition);
                isConOccur = true;
            }
            assStr = genAssignAEqB(updateEvent->desUpdateSlice,
                                                _isTempReq,
                                                updateEvent->srcUpdateValue);


            if (isConOccur){
                CbIfCxx& ifBlock = cb.addIf(condStr);
                ifBlock.addSt(assStr);
                if (!auxAssStr.empty()){
                    ifBlock.addSt(auxAssStr);
                }
            }else{
                cb.addSt(assStr);
                cb.addSt(auxAssStr);
            }
        }
    }

    std::string LogicSimEngine::genAssignAEqB(Slice desSlice, bool isDesTemp,
                                              Operable* srcOpr){
        assert(srcOpr != nullptr);
        assert(desSlice.stop <= _asb->getAssignSlice().stop);
        ////// src operand
        Slice           srcSlice          = srcOpr->getOperableSlice();
        LogicSimEngine* srcLogicSimEngine = srcOpr->getExactOperable().getLogicSimEngineFromOpr();
        Slice           baseSrcSlice      = srcOpr->getExactOperable().getOperableSlice();
        std::string     srcVarName        = getSrcOprFromOpr(srcOpr);

        assert(srcSlice.getSize() >= desSlice.getSize());
        ////// des operand
        Slice       baseDesSlice = _asb->getAssignSlice();
        std::string desVarName   = isDesTemp ? getTempVarName() : getVarName();

        if ((desSlice == baseDesSlice) &&
            (srcSlice == baseSrcSlice) &&
            (srcSlice == desSlice)){
            ///////// optimize
            return desVarName + " = " + srcVarName;
            }


        std::string ret;
        std::string desAStr = std::to_string(desSlice.start);
        std::string desBStr = std::to_string(desSlice.stop);
        /////////////////////// clear old data
        ret += desVarName + ".clear(" + desAStr +","+ desBStr +");\n";
        ret += "        ";
        ////////////////////// create new data
        ret += desVarName + " |= " + srcLogicSimEngine->genSlicedOprAndShift(desSlice, srcSlice, getValR_Type()) + ";\n";
        return ret;
    }

    std::string LogicSimEngine::genSrcOpr(){
        return getVarName();
    }

    std::string LogicSimEngine::genSlicedOprTo(Slice srcSlice, SIM_VALREP_TYPE desField){
        assert(srcSlice.checkValidSlice() &&
            (srcSlice.stop <= _asb->getAssignSlice().stop));
        ////// it will automatic shift to 0 index
        ////// des operand
        Slice baseSrcSlice = _asb->getAssignSlice();
        SIM_VALREP_TYPE baseType = getValR_Type();

        if ( (baseSrcSlice == srcSlice) &&
             (baseType     == desField)
        ){
            return getVarName();
        }

        std::string desTypeStr = SVT_toUnitType(desField);
        std::string aStr = std::to_string(srcSlice.start);
        std::string bStr = std::to_string(srcSlice.stop);
        return getVarName()+".slice(" +aStr+","+bStr+").cast<"+desTypeStr+">()";
    }

    std::string LogicSimEngine::genSlicedOprAndShift(Slice desSlice, Slice srcSlice, SIM_VALREP_TYPE desField){
        assert(srcSlice.checkValidSlice() &&
            (srcSlice.stop <= _asb->getAssignSlice().stop));
        assert(desSlice.getSize() <= srcSlice.getSize());

        Slice baseSrcSlice = _asb->getAssignSlice();
        SIM_VALREP_TYPE baseType = getValR_Type();

        if ((baseSrcSlice == srcSlice) &&
            (baseSrcSlice == desSlice) &&
            (baseType     == desField)
        ){ return getVarName(); }
        ///// may be our destination may have shorter bit width
        int actualSize = std::min(srcSlice.getSize(), desSlice.getSize());
        std::string shftToDesStart = std::to_string(desSlice.start);
        return "( " + genSlicedOprTo({srcSlice.start, srcSlice.start + actualSize}, desField)
              +".shift("+shftToDesStart+"))";
    }


    void LogicSimEngine::proxyBuildInit(){
        //// std::cout << _ident->getVarName() << std::endl;
        _asb->sortUpEventByPriority();
        for (UpdateEvent* updateEvent : _asb->getUpdateMeta()){
            if (updateEvent->srcUpdateCondition != nullptr){
                dep.push_back(
                    updateEvent->srcUpdateCondition->getLogicSimEngineFromOpr());
            }

            if (updateEvent->srcUpdateState != nullptr){
                dep.push_back(
                    updateEvent->srcUpdateState->getLogicSimEngineFromOpr());
            }

            if (updateEvent->srcUpdateValue != nullptr){
                dep.push_back(
                    updateEvent->srcUpdateValue->getLogicSimEngineFromOpr());
            }
        }
    }

    std::string LogicSimEngine::getVarName(){
        return _ident->getGlobalName() +
            (_ident->isUserVar() ? "_USER_" + _ident->getVarName() : "_SYS");
    }

    std::vector<std::string> LogicSimEngine::getRegisVarName(){
        return {getVarName()};
    }

    std::string LogicSimEngine::getTempVarName(){
        return getVarName() + TEMP_VAR_SUFFIX;
    }

    SIM_VALREP_TYPE LogicSimEngine::getValR_Type(){
        return getMatchSVT(_asb->getAssignSlice().getSize());
    }


    void LogicSimEngine::createGlobalVariable(CbBaseCxx& cb){
        std::string valSize = std::to_string(_asb->getAssignSlice().getSize());

        ////////"; will be auto add"

        SIM_VALREP_TYPE svt = getValR_Type();
        std::string typeStr = SVT_toType(svt);

        cb.addSt( typeStr + " " + getVarName() + " = " + std::to_string(_initVal), !_isTempReq);
        if (_isTempReq){
            cb.addSt(typeStr + " " + getTempVarName() + " = " + std::to_string(_initVal));
        }
    }


    void LogicSimEngine::createOp(CbBaseCxx& cb){
        ///////// build string
        cb.addCm(_ident->getGlobalName());
        _asb->sortUpEventByPriority();
        if (_isTempReq){
            cb.addSt(getTempVarName() + " = " + getVarName());
        }

        if (_asb->checkDesIsFullyAssignAndEqual()){
            genOpWithChainCondition(cb);
        }else{
            createOpWithSoleCondition(cb);
        }
    }

    void LogicSimEngine::createOpEndCycle2(CbBaseCxx& cb){
        if (_isTempReq){
            cb.addSt(getVarName() + " = " + getTempVarName());
        }
    }

    ///////////////////// proxyRetInit
    ///
    void LogicSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getVal(getVarName());
        proxyRep.setSize(_asb->getAssignSlice().getSize());
    }

    ValRepBase& LogicSimEngine::getProxyRep(){
        mfAssert(proxyRep.isInUsed(), "you might access the element that have not been tied with "
                 "registeration of proxy sim manager");
        return proxyRep;
    }




    void LogicSimEngine::genOpWithChainCondition(CbBaseCxx& cb, const std::string& auxAssStr){

           CbIfCxx* firstIfStatement = nullptr;

            int idx = 0;
            int maxUpdateEvent = _asb->getUpdateMeta().size();
            std::vector<UpdateEvent*> reversedUpdateEvents = _asb->getUpdateMeta();
            std::reverse(reversedUpdateEvents.begin(), reversedUpdateEvents.end());

            while (idx < maxUpdateEvent){

                std::vector<UpdateEvent*> updateEventGrp;
                updateEventGrp.push_back(reversedUpdateEvents[idx]);
                for (idx = idx + 1; idx < maxUpdateEvent; idx++){
                    UpdateEvent& curUpdateEvent = *reversedUpdateEvents[idx];
                    if (curUpdateEvent.priority == updateEventGrp[0]->priority){
                        if (curUpdateEvent.srcUpdateValue->isConstOpr() &&
                            updateEventGrp[0]->srcUpdateValue->isConstOpr() &&
                            (curUpdateEvent.srcUpdateValue->getConstOpr() ==
                                updateEventGrp[0]->srcUpdateValue->getConstOpr())
                        ){
                            updateEventGrp.push_back(reversedUpdateEvents[idx]);
                            continue; ///// grp updateEvent for const value
                        }
                        if (curUpdateEvent.srcUpdateValue ==
                            updateEventGrp[0]->srcUpdateValue){
                            updateEventGrp.push_back(reversedUpdateEvents[idx]);
                            continue; ///// grp updateEvent for other value
                        }
                    }
                    break;
                }

                assert(updateEventGrp[0]->srcUpdateValue->getOperableSlice().getSize() >=
                    updateEventGrp[0]->desUpdateSlice.getSize());


                std::string conStr;

                for (UpdateEvent* updateEvent : updateEventGrp){
                    bool isSubConOccur = false;
                    conStr += "(";
                    if (updateEvent->srcUpdateState != nullptr){
                        conStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateState);
                        isSubConOccur = true;
                    }

                    if (updateEvent->srcUpdateCondition != nullptr){
                        if (isSubConOccur){
                            conStr += " && ";
                        }
                        conStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateCondition);
                        isSubConOccur = true;
                    }

                    if (!isSubConOccur){
                        conStr += "true";
                    }
                    conStr += ")";
                    if ((updateEvent) != (*updateEventGrp.rbegin())){
                        conStr += " || ";
                    }
                }


                CbIfCxx* curIfStatement = nullptr;
                if (firstIfStatement == nullptr){
                    firstIfStatement = &cb.addIf(conStr);
                    curIfStatement   = firstIfStatement;
                }else{
                    curIfStatement   = &firstIfStatement->addElif(conStr);
                }


                curIfStatement->addSt(genAssignAEqB(updateEventGrp[0]->desUpdateSlice, _isTempReq,
                                      updateEventGrp[0]->srcUpdateValue));
                if (!auxAssStr.empty()){
                    curIfStatement->addSt(auxAssStr);
                }
            }

        }
}