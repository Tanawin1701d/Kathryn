//
// Created by tanawin on 18/7/2024.
//

#include "memAgentSim.h"

#include "model/hwComponent/memBlock/MemBlockAgent.h"
#include "model/hwComponent/memBlock/MemBlock.h"
#include "sim/modelSimEngine/hwComponent/abstract/genHelper.h"

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


    void MemEleHolderSimEngine::createGlobalVariable(CbBaseCxx& cb){

        //std::string valSize = std::to_string(_asb->getAssignSlice().getSize());
        if(_master->isWriteMode()){
            ////std::string indexerSize = std::to_string(_master->getExactIndexSize());

            cb.addSt("ull " + getVarName()    + " = 0", false);
            cb.addSt("ull " + getIsSetVar()   + " = 0", false);
            cb.addSt("ull " + getIndexerVar() + " = 0", true);
        }

        if (_reqGlobDec && _master->isReadMode()){
            cb.addSt("ull " + getVarName() + " = " + std::to_string(_initVal));
        }
    }

    void MemEleHolderSimEngine::createLocalVariable(CbBaseCxx& cb){
        std::string valSize = std::to_string(_asb->getAssignSlice().getSize());
        if((!_reqGlobDec) && _master->isReadMode()){
            cb.addSt("ull " + getVarName() + " = " + std::to_string(_initVal));
        }
    }


    void MemEleHolderSimEngine::createOp(CbBaseCxx& cb){
        _asb->sortUpEventByPriority();
        if (_master->isWriteMode()){ ///// write mode
            return createOpWriteMode(cb);
        }

        if (_master->isReadMode()){
            return createOpReadMode(cb);
        }

        assert(false);

    }


    std::string MemEleHolderSimEngine::getIsSetVar(){
        return getVarName() + IS_SET_SUFFIX;
    }

    std::string MemEleHolderSimEngine::getIndexerVar(){
        return getVarName() + INDEXER_SUFFIX;
    }


    void MemEleHolderSimEngine::createOpReadMode(CbBaseCxx& cb){
        ///
        /////// read mode
        ///
        cb.addCm(_ident->getGlobalName() + "  readMode");

        cb.addSt( getVarName() + " = " +
         _master->_master->getSimEngine()->getVarName() +
         "["+ getSlicedSrcOprFromOpr(_master->_indexer) + "]");

    }

    void MemEleHolderSimEngine::createOpWriteMode(CbBaseCxx& cb){

        ////////////// calculate aux Val
        std::string auxAssVal;
        auxAssVal += getIsSetVar() + " = 1; ";
        ////////////// assign index value
        auxAssVal += getIndexerVar() + " = " +
                  getSlicedSrcOprFromOpr(_master->_indexer);

        ///////// build string
        cb.addCm(_ident->getGlobalName());
        assert(_asb->checkDesIsFullyAssignAndEqual());
        createOpWithSoleCondition(cb, auxAssVal);
    }


    void MemEleHolderSimEngine::createOpEndCycle(CbBaseCxx& cb){

        if (_master->isWriteMode()){
            cb.addCm(_ident->getGlobalName());

            CbIfCxx&  ifBlock = cb.addIf(getIsSetVar());

            ///////////// add value
            ifBlock.addSt(
            _master->_master->getSimEngine()->getVarName() +
            "[" + getIndexerVar() + "] = " + getVarName()
            );
            ///////////// reset is set
            ifBlock.addSt(getIsSetVar() + " = 0");
        }

    }









}
