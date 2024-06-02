//
// Created by tanawin on 31/5/2024.
//
#include "logicSimEngine.h"



namespace kathryn{


    LogicSimEngine::LogicSimEngine(Assignable* asb, IdentBase* ident,
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
        assert(_vcdSigType != VCD_SIG_TYPE::VST_DUMMY);
        ///////// fill asb  to system to support topology sort
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

    std::string getVarNameFromOpr(Operable* opr){
        assert(opr != nullptr);
        return opr->getExactOperable().getLogicSimEngineFromOpr()->getVarName();
    }


    std::string LogicSimEngine::createVariable(){

        std::string valSize = std::to_string(_asb->getAssignSlice().getSize());

        return "ValRep<"+valSize+"> " + getVarName() + " = " + std::to_string(_initVal) + ";\n"
                + (_isTempReq ?
                   "ValRep<"+valSize+"> " + getVarName() + TEMP_VAR_SUFFIX + " = " + std::to_string(_initVal) + ";\n":
                   "");
    }

    std::string LogicSimEngine::createOp(){

        ///////// build string
        std::string retStr = "{ /////" + _ident->getGlobalName() + "\n";

        /////////// we build from low priority to high priority
        for (UpdateEvent* updateEvent: _asb->getUpdateMeta()){

            ////
            ///check integrity
            ///
            assert(updateEvent->srcUpdateValue->getOperableSlice().getSize() ==
                   updateEvent->desUpdateSlice.getSize());

            retStr += "         if ( ";
            bool isConOccur = false;
            if (updateEvent->srcUpdateCondition != nullptr){
                isConOccur = true;
                retStr += getVarNameFromOpr(updateEvent->srcUpdateCondition);
            }

            if (updateEvent->srcUpdateState != nullptr){
                if (isConOccur){
                    retStr += " && ";
                }
                retStr += getVarNameFromOpr(updateEvent->srcUpdateState);
            }

            if (!isConOccur){
                retStr += "true";
            }

            retStr += "){\n         ";
            retStr += "         ";
            retStr += getVarName() + (_isTempReq ? TEMP_VAR_SUFFIX: "") +
                ".updateOnSlice<"+ std::to_string(updateEvent->desUpdateSlice.start) + "," +
                                   std::to_string(updateEvent->desUpdateSlice.stop) + ">(" ;
            retStr += getVarNameFromOpr(updateEvent->srcUpdateValue);
            retStr += ".sliceAndShift<"+std::to_string(updateEvent->srcUpdateValue->getOperableSlice().start) + "," +
                                        std::to_string(updateEvent->srcUpdateValue->getOperableSlice().stop ) + "," +
                                        std::to_string(updateEvent->desUpdateSlice.start)
                                       +">());\n";
            retStr += "         }\n";
        }

        retStr += "     }\n";
        return retStr;
    }

    std::string LogicSimEngine::createMemorizeOp(){
        if (_isTempReq){
            return getVarName() + TEMP_VAR_SUFFIX + " = " + getVarName() + ";\n";
        }
        return "";
    }







}

