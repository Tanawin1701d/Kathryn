//
// Created by tanawin on 31/5/2024.
//

#include "model/simIntf/base/proxyEventBase.h"
#include "logicSimEngine.h"

#include <util/numberic/numConvert.h>

#include "sim/controller/simController.h"



namespace kathryn{


    LogicSimEngine::LogicSimEngine(Assignable* asb, Identifiable* ident,
                                   VCD_SIG_TYPE sigType, bool isTempReq,
                                   ull initVal):
    _asb(asb),
    _ident(ident),
    _vcdSigType(sigType),
    _isTempReq(isTempReq),
    _initVal(initVal)
    {
        assert(asb != nullptr);
        assert(ident != nullptr);
        //assert(_vcdSigType != VCD_SIG_TYPE::VST_DUMMY);
        ///////// fill asb  to system to support topology sort
    }

    ull LogicSimEngine::createMask(Slice maskSlice){
        assert((maskSlice.checkValidSlice()) && (maskSlice.stop <= bitSizeOfUll));
        assert(maskSlice.start < bitSizeOfUll);
        int size = maskSlice.getSize();
        return (size == bitSizeOfUll) ? -1 : ((((ull)1) << size) - 1);
    }


    std::string LogicSimEngine::getSrcOprFromOpr(Operable* opr){
        assert(opr != nullptr);
        return opr->getExactOperable().
                getLogicSimEngineFromOpr()->genSrcOpr();
    }

    std::string LogicSimEngine::getSlicedSrcOprFromOpr(Operable* opr){
        assert(opr != nullptr);
        LogicSimEngine* simEngine   = opr->getLogicSimEngineFromOpr();
        Slice           neededSlice = opr->getOperableSlice();
        ////// slice to operabe slice
        return simEngine->genSlicedOprTo(neededSlice);
    }

    std::string getSlicedAndShiftSrcOprFromOpr(Operable* opr, Slice desSlice){
        assert(opr != nullptr);
        LogicSimEngine* simEngine = opr->getLogicSimEngineFromOpr();
        Slice           neededSlice = opr->getOperableSlice();
        ////// slice to operabe slice
        return simEngine->genSlicedOprAndShift(desSlice, neededSlice);
    }


    std::string LogicSimEngine::genOpWithSoleCondition(const std::string& auxAssStr){
        std::string retStr;
        for (UpdateEvent* updateEvent: _asb->getUpdateMeta()){
            ////
            ///check integrity
            ///
            assert(updateEvent->srcUpdateValue->getOperableSlice().getSize() >=
                   updateEvent->desUpdateSlice.getSize());

            retStr += "         if ( ";
            bool isConOccur = false;

            if (updateEvent->srcUpdateState != nullptr){

                retStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateState);
                isConOccur = true;
            }

            if (updateEvent->srcUpdateCondition != nullptr){
                if (isConOccur){
                    retStr += " && ";
                }
                retStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateCondition);
                isConOccur = true;

            }

            if (!isConOccur){
                retStr += "true";
            }

            retStr += "){\n         ";
            retStr += "         ";
            retStr += genAssignAEqB(updateEvent->desUpdateSlice, _isTempReq, updateEvent->srcUpdateValue) + "\n";
            if (!auxAssStr.empty()){
                retStr += auxAssStr + "\n";
            }
            retStr += "         }\n";
        }
        return retStr;
    }

    std::string LogicSimEngine::genOpWithChainCondition(const std::string& auxAssStr){

        bool secondTime = false;
        std::string retStr;

        int idx = 0;
        int maxUpdateEvent = _asb->getUpdateMeta().size();
        std::vector<UpdateEvent*> reversedUpdateEvents = _asb->getUpdateMeta();
        std::reverse(reversedUpdateEvents.begin(), reversedUpdateEvents.end());

        while (idx < maxUpdateEvent){
            std::vector<UpdateEvent*> updateEventGrp;
            updateEventGrp.push_back(reversedUpdateEvents[idx]);
            for (idx = idx+1; idx < maxUpdateEvent; idx++){
                UpdateEvent& curUpdateEvent = *reversedUpdateEvents[idx];
                if (curUpdateEvent.priority == updateEventGrp[0]->priority){
                    if ( curUpdateEvent.srcUpdateValue->isConstOpr() &&
                         updateEventGrp[0]->srcUpdateValue->isConstOpr() &&
                          (curUpdateEvent.srcUpdateValue->getConstOpr() ==
                          updateEventGrp[0]->srcUpdateValue->getConstOpr())
                    ){
                        updateEventGrp.push_back(reversedUpdateEvents[idx]);
                        continue; ///// grp updateEvent for const value
                    }
                    if(curUpdateEvent.srcUpdateValue ==
                             updateEventGrp[0]->srcUpdateValue){
                        updateEventGrp.push_back(reversedUpdateEvents[idx]);
                        continue; ///// grp updateEvent for other value
                    }
                }
                break;
            }

            assert(updateEventGrp[0]->srcUpdateValue->getOperableSlice().getSize() >=
                   updateEventGrp[0]->desUpdateSlice.getSize());


            retStr += (secondTime ? "         else if ( " : "         if ( ");


            for(UpdateEvent* updateEvent: updateEventGrp){
                retStr += "(";
                bool isSubConOccur = false;
                if (updateEvent->srcUpdateState != nullptr){
                    retStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateState);
                    isSubConOccur = true;
                }

                if (updateEvent->srcUpdateCondition != nullptr){
                    if (isSubConOccur){
                        retStr += " && ";
                    }
                    retStr += getSlicedSrcOprFromOpr(updateEvent->srcUpdateCondition);
                    isSubConOccur = true;
                }

                if (!isSubConOccur){
                    retStr += "true";
                }
                retStr += ")\n          ";
                if ((updateEvent) != (*updateEventGrp.rbegin())){
                    retStr += " || ";
                }
            }

            retStr += "){\n         ";
            retStr += "         ";
            retStr += genAssignAEqB(updateEventGrp[0]->desUpdateSlice, _isTempReq, updateEventGrp[0]->srcUpdateValue) + "\n";
            if (!auxAssStr.empty()){
                retStr += auxAssStr + "\n";
            }
            retStr += "         }\n";
            secondTime = true;

        }

        return retStr;

    }

    std::string LogicSimEngine::genAssignAEqB(Slice desSlice, bool isDesTemp,
                                              Operable* srcOpr){
        assert(srcOpr != nullptr);
        assert(desSlice.stop <= _asb->getAssignSlice().stop);
        ////// src operand
        Slice srcSlice            = srcOpr->getOperableSlice();
        LogicSimEngine*
              srcLogicSimEngine   = srcOpr->getExactOperable().getLogicSimEngineFromOpr();
        Slice baseSrcSlice        = srcOpr->getExactOperable().getOperableSlice();
        std::string srcVarName    = getSrcOprFromOpr(srcOpr);

        assert(srcSlice.getSize() >= desSlice.getSize());
        ////// des operand
        Slice       baseDesSlice  = _asb->getAssignSlice();
        std::string desVarName    = isDesTemp ? getTempVarName() : getVarName();

        if ((desSlice == baseDesSlice) &&
            (srcSlice == baseSrcSlice) &&
            (srcSlice == desSlice    )){
            ///////// optimize
            return desVarName + " = " + srcVarName + ";";
        }


        std::string ret;
        std::string mask = cvtNum2HexStr(~(createMask(desSlice) << desSlice.start));
        /////////////////////// clear old data
        ret += desVarName + " &= " + mask + ";\n";
        ret += "        ";
        ////////////////////// create new data
        ret += desVarName + " |= " + srcLogicSimEngine->genSlicedOprAndShift(desSlice,srcSlice)  + ";\n";
        return ret;
    }

    std::string LogicSimEngine::genSrcOpr(){
        return getVarName();
    }

    std::string LogicSimEngine::genSlicedOprTo(Slice srcSlice){
        assert( srcSlice.checkValidSlice() &&
               (srcSlice.stop <= _asb->getAssignSlice().stop));
        ////// it will automatic shift to 0 index
        ////// des operand
        Slice       baseSrcSlice  = _asb->getAssignSlice();
        std::string desVarName    = getVarName();

        if (baseSrcSlice == srcSlice){
            return getVarName();
        }
        /// "( %h & ( %s >> %d ) )"
        std::string mask      = cvtNum2HexStr(createMask(srcSlice));
        std::string value     = getVarName();
        std::string shftStart = std::to_string(srcSlice.start);
        return "( " + mask + " & ( " + value + " >> " + shftStart + " ))";
    }

    std::string LogicSimEngine::genSlicedOprAndShift(Slice desSlice, Slice srcSlice){
        assert( srcSlice.checkValidSlice() &&
               (srcSlice.stop <= _asb->getAssignSlice().stop));
        assert(desSlice.getSize() <= srcSlice.getSize());

        Slice       baseSrcSlice  = _asb->getAssignSlice();
        std::string desVarName    = getVarName();

        if ( (baseSrcSlice == srcSlice) &&
             (baseSrcSlice == desSlice)
        ){ return getVarName();}
        /// "( %h & ( %s >> %d ) )"
        int actualSize = std::min(srcSlice.getSize(), desSlice.getSize());
        std::string shftToDesStart = std::to_string(desSlice.start);
        return "( " + genSlicedOprTo({srcSlice.start, srcSlice.start + actualSize})
               + "<< " + shftToDesStart + ")";

    }


    void LogicSimEngine::proxyBuildInit(){

        _asb->sortUpEventByPriority();
        for (UpdateEvent* updateEvent: _asb->getUpdateMeta()){
            if (updateEvent->srcUpdateCondition != nullptr){
                dep.push_back(
                    updateEvent->srcUpdateCondition->getLogicSimEngineFromOpr());
            }

            if(updateEvent->srcUpdateState != nullptr){
                dep.push_back(
                    updateEvent->srcUpdateState->getLogicSimEngineFromOpr());
            }

            if(updateEvent->srcUpdateValue != nullptr){
                dep.push_back(
                    updateEvent->srcUpdateValue->getLogicSimEngineFromOpr());
            }
        }
    }

    std::string LogicSimEngine::getVarName(){
        return _ident->getGlobalName() +
               (_ident->isUserVar() ?
                   "_USER_" + _ident->getVarName() :
                   "_SYS");
    }

    std::vector<std::string> LogicSimEngine::getRegisVarName(){
        return {getVarName()};
    }

    std::string LogicSimEngine::getTempVarName(){
        return getVarName() + TEMP_VAR_SUFFIX;
    }

    std::string LogicSimEngine::createGlobalVariable(){

        std::string valSize = std::to_string(_asb->getAssignSlice().getSize());

        return "ull " + getVarName() + " = " + std::to_string(_initVal) + "; "
                + (_isTempReq ?
                   "ull " + getVarName() + TEMP_VAR_SUFFIX + " = " + std::to_string(_initVal) + ";":
                   "") + "\n";
    }



    std::string LogicSimEngine::createOp(){

        ///////// build string
        std::string retStr = "      { /////" + _ident->getGlobalName() + "\n";
        _asb->sortUpEventByPriority();
        if (_isTempReq){
            retStr += "         " + getVarName() + TEMP_VAR_SUFFIX + " = " + getVarName()  + ";\n";
        }
        /////////// we build from low priority to high priority
        if (_asb->checkDesIsFullyAssignAndEqual()){
            retStr += genOpWithChainCondition();
        }else{
            retStr += genOpWithSoleCondition();
        }

        retStr += "     }\n";
        return retStr;
    }

    std::string LogicSimEngine::createOpEndCycle2(){
        if (_isTempReq){
            return "    " + getVarName() + " = " + getTempVarName() + ";\n";
        }
        return "";
    }
    ///////////////////// proxyRetInit
    ///
    void LogicSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = new ValRepBase(
                        _asb->getAssignSlice().getSize(),
                        *modelSimEvent->getVal(getVarName()));
    }

    ValRepBase* LogicSimEngine::getProxyRep(){
        mfAssert(proxyRep != nullptr, "you might access the element that have not been tied with "
                                      "registeration of proxy sim manager");
        return proxyRep;
    }








}

