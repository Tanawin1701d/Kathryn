//
// Created by tanawin on 31/5/2024.
//

#include "logicSimEngine.h"

#include "genHelper.h"
#include "sim/controller/simController.h"


namespace kathryn{
    LogicSimEngine::LogicSimEngine(Assignable* asb, Identifiable* ident, Operable* opr,
                                   VCD_SIG_TYPE sigType, bool isTempReq,
                                   ull initVal):
        _asb(asb),
        _ident(ident),
        _opr(opr),
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
                condStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateState).toString();
                isConOccur = true;
            }
            if (updateEvent->srcUpdateCondition != nullptr){
                if (isConOccur){
                    condStr += " && ";
                }
                condStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateCondition).toString();
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
        if (_ident->getGlobalId() == 103){
            std::cout << "got 103" << std::endl;
        }
        assert(srcOpr != nullptr);
        assert(desSlice.stop <= _asb->getAssignSlice().stop);
        ////// src operand
        Slice           srcSlice          = srcOpr->getOperableSlice();
        Slice           baseSrcSlice      = srcOpr->getExactOperable().getOperableSlice();
        ValR            srcVar            = getSrcOprFromOpr(srcOpr);

        assert(srcSlice.getSize() >= desSlice.getSize());
        ////// des operand
        Slice       baseDesSlice = _asb->getAssignSlice();
        ValR        desVar       = isDesTemp ? getTempValRep() : getValRep();

        if ((desSlice == baseDesSlice) &&
            (srcSlice == baseSrcSlice) &&
            (srcSlice == desSlice)){
            ///////// optimize
            return desVar.eq(srcVar).toString();
        }


        std::string ret;
        std::string desAStr = std::to_string(desSlice.start);
        std::string desBStr = std::to_string(desSlice.stop);
        /////////////////////// clear old data
        ret += desVar.eq(desVar.clear(desSlice)).toString() + ";\n";
        ret += "        ";
        ////////////////////// create new data

        ValR fillVal = getSlicedAndShiftSrcOprFromOpr(srcOpr, desSlice, getValR_Type());
        ret += desVar
        .eq(desVar.partialOr(fillVal))
        .toString() + ";";

        return ret;
    }

    ValR LogicSimEngine::genSrcOpr(){
        return getValRep();
    }

    ValR LogicSimEngine::genSlicedOprTo(Slice srcSlice, SIM_VALREP_TYPE_ALL desField){
        assert(srcSlice.checkValidSlice() &&
            (srcSlice.stop <= _asb->getAssignSlice().stop));
        ////// it will automatic shift to 0 index
        ////// des operand
        Slice baseSrcSlice = _asb->getAssignSlice();
        SIM_VALREP_TYPE_ALL baseType = getValR_Type();

        if ( (baseSrcSlice == srcSlice) &&
             (baseType     == desField)
        ){
            return getValRep();
        }

        std::string aStr = std::to_string(srcSlice.start);
        std::string bStr = std::to_string(srcSlice.stop);
        return getValRep()
                .slice(srcSlice)
                .cast(desField, srcSlice.getSize());

    }

    ValR LogicSimEngine::genSlicedOprAndShift(Slice desSlice, Slice srcSlice, SIM_VALREP_TYPE_ALL desField){
        assert(srcSlice.checkValidSlice() &&
            (srcSlice.stop <= _asb->getAssignSlice().stop));
        assert(desSlice.getSize() <= srcSlice.getSize());

        Slice baseSrcSlice = _asb->getAssignSlice();
        SIM_VALREP_TYPE_ALL baseType = getValR_Type();

        if ((baseSrcSlice == srcSlice) &&
            (baseSrcSlice == desSlice) &&
            (baseType     == desField)
        ){ return getValRep();}
        ///// may be our destination may have shorter bit width
        int actualSize = std::min(srcSlice.getSize(), desSlice.getSize());
        return genSlicedOprTo({srcSlice.start, srcSlice.start + actualSize}, desField).
                shift(desSlice.start);
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

    ValR LogicSimEngine::getValRep(){
        std::string name = _ident->getGlobalName() +
            (_ident->isUserVar() ? "_USER_" + _ident->getVarName() : "_SYS");

        int size = _asb->getAssignSlice().getSize();
        SIM_VALREP_TYPE_ALL valType = getValR_Type();
        return {valType, size, name};
    }

    std::vector<std::string> LogicSimEngine::getRegisVarName(){
        return {getValRep().getData()};
    }

    ValR LogicSimEngine::getTempValRep(){
        ValR base = getValRep();
        base.setData(base.getData() + TEMP_VAR_SUFFIX);
        return base;
    }

    SIM_VALREP_TYPE_ALL LogicSimEngine::getValR_Type(){
        return SIM_VALREP_TYPE_ALL(_asb->getAssignSlice().getSize());
    }


    void LogicSimEngine::createGlobalVariable(CbBaseCxx& cb){
        std::string valSize = std::to_string(_asb->getAssignSlice().getSize());

        ////////"; will be auto add"

        cb.addSt( getValRep().buildVar(_initVal), !_isTempReq);
        if (_isTempReq){
            cb.addSt(getTempValRep().buildVar(_initVal));
        }
    }


    void LogicSimEngine::createOp(CbBaseCxx& cb){
        ///////// build string
        cb.addCm(_ident->getGlobalName());
        _asb->sortUpEventByPriority();
        if (_isTempReq){
            cb.addSt( //// build temp variable first
                getTempValRep().eq(getValRep()).toString()
            );
        }

        if (_asb->checkDesIsFullyAssignAndEqual()){
            genOpWithChainCondition(cb);
        }else{
            createOpWithSoleCondition(cb);
        }
    }

    void LogicSimEngine::createOpEndCycle2(CbBaseCxx& cb){
        if (_isTempReq){
            cb.addSt(
                getValRep().eq(getTempValRep()).toString()
            );
        }
    }

    ///////////////////// proxyRetInit
    ///
    void LogicSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getVal(getValRep().toString());
        proxyRep.setSize(_asb->getAssignSlice().getSize());
        if (getValR_Type().type == SVT_U64M){
            assert(getValR_Type().subType > 0);
            proxyRep.setContinLength(getValR_Type().subType);
        }
        ///// we cache it in operable
        _opr->initValRep(proxyRep);
    }



    ValRepBase& LogicSimEngine::getProxyRep(){
        mfAssert(proxyRep.isInUsed(), "you might access the element that have not been tied with "
                 "registeration of proxy sim manager");
        return proxyRep;
    }




    void LogicSimEngine::genOpWithChainCondition(CbBaseCxx& cb, const std::string& auxAssStr){

           CbIfCxx* firstIfStatement = nullptr;

            int idx = 0;
            int maxUpdateEvent = static_cast<int>(_asb->getUpdateMeta().size());
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
                        conStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateState).toString();
                        isSubConOccur = true;
                    }

                    if (updateEvent->srcUpdateCondition != nullptr){
                        if (isSubConOccur){
                            conStr += " && ";
                        }
                        conStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateCondition).toString();
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