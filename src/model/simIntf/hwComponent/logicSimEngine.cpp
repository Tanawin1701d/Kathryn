//
// Created by tanawin on 31/5/2024.
//

#include "model/simIntf/base/proxyEventBase.h"
#include "logicSimEngine.h"
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


    std::string LogicSimEngine::getVarNameFromOpr(Operable* opr){
        assert(opr != nullptr);
        return opr->getExactOperable().getLogicSimEngineFromOpr()->getVarName();
    }

    std::string LogicSimEngine::getSliceStringFromOpr(Operable* opr, int fixLength){
        assert(opr != nullptr);
        LogicSimEngine* simEngine = opr->getLogicSimEngineFromOpr();
        Slice           neededSlice = opr->getOperableSlice();
        if (fixLength == -1){
            //////// slice to operabe slice
            return simEngine->genSliceTo(neededSlice);
        }
        return simEngine->genSliceToWithFixSize(neededSlice, fixLength);

    }

    std::string LogicSimEngine::genAssignAEqB(Slice desSlice, bool isDesTemp,
                                              Operable* srcOpr, bool isShinkSrc){
        assert(srcOpr != nullptr);

        ////// src operand
        Slice srcSlice            = srcOpr->getOperableSlice();
        Slice shinkSrcSlice       = srcSlice.getSubSliceWithShinkMsb({0,
                                                                        desSlice.getSize()});
        Slice baseSrcSlice        = srcOpr->getExactOperable().getOperableSlice();
        std::string srcVarName    = getVarNameFromOpr(srcOpr);

        assert(srcSlice.getSize() >= desSlice.getSize());
        ////// des operand
        Slice       baseDesSlice  = _asb->getAssignSlice();
        std::string desVarName    = isDesTemp ? getTempVarName() : getVarName();

        if ((desSlice == baseDesSlice) &&
            (srcSlice == baseSrcSlice) &&
            (srcSlice == desSlice    ) &&
            ( (!isShinkSrc) || (isShinkSrc && (srcSlice == shinkSrcSlice))   )
        ){
            ///////// optimize
            return desVarName + " = " + srcVarName + ";";
        }

        int srcStop = (isShinkSrc) ? shinkSrcSlice.stop : srcSlice.stop;
        return desVarName + ".updateOnSlice<" + std::to_string(desSlice.start) + ", " + std::to_string(desSlice.stop) + ">(" +
               srcVarName + ".sliceAndShift<" + std::to_string(srcSlice.start) + ", " + std::to_string(srcStop) + ", "
                                              + std::to_string(desSlice.start) + ">());";
    }

    std::string LogicSimEngine::genSliceTo(Slice desSlice){
        ////// it will automatic shift to 0 index
        ////// des operand
        Slice       baseDesSlice  = _asb->getAssignSlice();
        std::string desVarName    = getVarName();

        if (baseDesSlice == desSlice){
            return getVarName();
        }
        return getVarName() + ".slice<" + std::to_string(desSlice.start) + ", " +
                                          std::to_string(desSlice.stop) + ">()";

    }

    std::string LogicSimEngine::genSliceToWithFixSize(Slice desSlice, int fixLength){
        ////// it will automatic shift to 0 index
        ////// des operand
        Slice       baseDesSlice  = _asb->getAssignSlice();
        std::string desVarName    = getVarName();

        if ( (baseDesSlice == desSlice) &&
             (fixLength == baseDesSlice.getSize())
        ){
            return getVarName();
        }
        return getVarName() + ".slice<" + std::to_string(desSlice.start) +  ", " +
                                          std::to_string(desSlice.stop)  +  ", " +
                                          std::to_string(fixLength)      +  ">()";

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

    std::string LogicSimEngine::createVariable(){

        std::string valSize = std::to_string(_asb->getAssignSlice().getSize());

        return "ValRep<"+valSize+"> " + getVarName() + " = " + std::to_string(_initVal) + "; "
                + (_isTempReq ?
                   "ValRep<"+valSize+"> " + getVarName() + TEMP_VAR_SUFFIX + " = " + std::to_string(_initVal) + ";":
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
        for (UpdateEvent* updateEvent: _asb->getUpdateMeta()){

            ////
            ///check integrity
            ///
            assert(updateEvent->srcUpdateValue->getOperableSlice().getSize() >=
                   updateEvent->desUpdateSlice.getSize());

            retStr += "         if ( ";
            bool isConOccur = false;
            if (updateEvent->srcUpdateCondition != nullptr){
                retStr += getSliceStringFromOpr(updateEvent->srcUpdateCondition);
                isConOccur = true;

            }

            if (updateEvent->srcUpdateState != nullptr){
                if (isConOccur){
                    retStr += " && ";
                }
                retStr += getSliceStringFromOpr(updateEvent->srcUpdateState);
                isConOccur = true;
            }

            if (!isConOccur){
                retStr += "true";
            }

            retStr += "){\n         ";
            retStr += "         ";
            retStr += genAssignAEqB(updateEvent->desUpdateSlice, _isTempReq, updateEvent->srcUpdateValue, true) + "\n";
            retStr += "         }\n";
        }

        retStr += "     }\n";
        return retStr;
    }

    std::string LogicSimEngine::createOpEndCycle(){
        if (_isTempReq){
            return "    " + getVarName() + " = " + getTempVarName() + ";\n";
        }
        return "";
    }
    ///////////////////// proxyRetInit
    ///
    void LogicSimEngine::proxyRetInit(ProxySimEventBase* modelSimEvent){
        proxyRep = modelSimEvent->getValRep(getVarName());
    }

    ValRepBase* LogicSimEngine::getProxyRep(){
        assert(proxyRep != nullptr);
        return proxyRep;
    }








}

