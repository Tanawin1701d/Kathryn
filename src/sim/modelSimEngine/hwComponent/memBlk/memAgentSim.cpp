//
// Created by tanawin on 18/7/2024.
//

#include "memAgentSim.h"

#include "model/hwComponent/memBlock/MemBlockAgent.h"
#include "model/hwComponent/memBlock/MemBlock.h"

namespace kathryn{



    /**
     *
     * MemEleSimEngine
     *
     * */

    MemEleHolderSimEngine::MemEleHolderSimEngine(MemBlockEleHolder* master):
    LogicSimEngine(master, master, VST_WIRE, false, 0),
    _master(master){
        assert(master != nullptr);
    }

    void MemEleHolderSimEngine::proxyBuildInit(){
        assert(_master->_indexer != nullptr);
        dep.push_back(_master->_indexer->getLogicSimEngineFromOpr());

        ///// write mode
        if (_master->isWriteMode()){
            LogicSimEngine::proxyBuildInit();
        }
    }


    std::string MemEleHolderSimEngine::createGlobalVariable(){

        std::string valSize = std::to_string(_asb->getAssignSlice().getSize());
        if(_master->isWriteMode()){
            std::string indexerSize = std::to_string(_master->getExactIndexSize());
            return "ull " + getVarName()    + " = 0;"
            +      "ull " + getIsSetVar()   + " = 0;"
            +      "ull " + getIndexerVar() + " = 0;";
        }
        return "";
    }

    std::string MemEleHolderSimEngine::createLocalVariable(){
        std::string valSize = std::to_string(_asb->getAssignSlice().getSize());
        if(_master->isReadMode()){
            return "ull " + getVarName() + " = " + std::to_string(_initVal) + "; \n";
        }
        return "";
    }


    std::string MemEleHolderSimEngine::createOp(){
        _asb->sortUpEventByPriority();
        if (_master->isWriteMode()){ ///// write mode
            return createOpWriteMode();
        }

        if (_master->isReadMode()){
            return createOpReadMode();
        }

        assert(false);

    }


    std::string MemEleHolderSimEngine::getIsSetVar(){
        return getVarName() + IS_SET_SUFFIX;
    }

    std::string MemEleHolderSimEngine::getIndexerVar(){
        return getVarName() + INDEXER_SUFFIX;
    }


    std::string MemEleHolderSimEngine::createOpReadMode(){
        ///
        /////// read mode
        ///
        std::string retStr = "      { ///////" + _ident->getGlobalName() + "  readMode\n";

        retStr += "     ";
        retStr += getVarName() + " = ";
        retStr += _master->_master->getSimEngine()->getVarName();
        retStr += "["+ getSlicedSrcOprFromOpr(_master->_indexer) + "];\n";
        retStr += "     }\n";
        return retStr;
    }

    std::string MemEleHolderSimEngine::createOpWriteMode(){

        std::string auxAssVal;
        auxAssVal += "         ";
        auxAssVal += getIsSetVar() + " = 1;\n";
        ////////////// assign index value
        auxAssVal += "         ";
        auxAssVal += getIndexerVar() + " = " +
                  getSlicedSrcOprFromOpr(_master->_indexer);
        auxAssVal += ";\n";

        ///////// build string
        std::string retStr = "      { /////" + _ident->getGlobalName() + "\n";
        assert(_asb->checkDesIsFullyAssignAndEqual());
        retStr += genOpWithChainCondition(auxAssVal);
        retStr += "     }\n";
        return retStr;
    }


    std::string MemEleHolderSimEngine::createOpEndCycle(){

        if (_master->isWriteMode()){
            std::string retStr= "      { /////" + _ident->getGlobalName() + "\n";


            retStr += "          if ( " + getIsSetVar() + " ){\n";

            ///////////// add value
            retStr += "         ";
            retStr += _master->_master->getSimEngine()->getVarName();
            retStr += "[" + getIndexerVar() + "] = " + getVarName() + ";\n";
            ///////////// reset is set
            retStr += "         ";
            retStr += getIsSetVar();
            retStr += " = 0;\n";

            retStr += "         }\n";
            retStr += "     }\n";

            return retStr;
        }



        return "";

    }









}