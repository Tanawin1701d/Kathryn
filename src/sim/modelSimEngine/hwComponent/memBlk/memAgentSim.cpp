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
    LogicSimEngine(master, master, master, VST_WIRE, false, 0),
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

        if(_master->isWriteMode()){

            cb.addSt(getValRep    ().buildVar(_initVal) , false);
            cb.addSt(getIsSetVar  ().buildVar(0)   , false);
            cb.addSt(getIndexerVar().buildVar(0)   , true);
        }

        if (_reqGlobDec && _master->isReadMode()){
            cb.addSt(getValRep().buildVar(_initVal));
        }
    }

    void MemEleHolderSimEngine::createLocalVariable(CbBaseCxx& cb){
        if((!_reqGlobDec) && _master->isReadMode()){
            cb.addSt(getValRep().buildVar(_initVal));
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


    ValR MemEleHolderSimEngine::getIsSetVar(){
        std::string name = getValRep()._data + IS_SET_SUFFIX;
        return {SIM_VALREP_TYPE_ALL(1), 1, name};
    }

    ValR MemEleHolderSimEngine::getIndexerVar(){
        std::string name = getValRep()._data + INDEXER_SUFFIX;
        int         size = _master->getExactIndexSize();
        return {SIM_VALREP_TYPE_ALL(size), size, name};
    }


    void MemEleHolderSimEngine::createOpReadMode(CbBaseCxx& cb){
        ///
        /////// read mode
        ///
        cb.addCm(_ident->getGlobalName() + "  readMode");
        ////// we are so sure that it is the same as the width
        ValR memValR = _master->_master->getSimEngine()->getValRep();
        ////////// we must fix indexer type to match main memory
        ValR indexer = getSlicedSrcOprFromOpr(_master->_indexer,getIndexerVar()._valType);

        cb.addSt(getValRep().eq(memValR.index(indexer))
        .toString());
    }

    void MemEleHolderSimEngine::createOpWriteMode(CbBaseCxx& cb){

        ValR setterEq  = getIsSetVar().eq(ValR(SIM_VALREP_TYPE_ALL(1), 1, "1"));
        ValR indexer   = getSlicedSrcOprFromOpr(_master->_indexer,getIndexerVar()._valType);
        ////// index and push to local variable
        ValR indexerEq = getIndexerVar().eq(indexer);
        std::string auxAssVal = setterEq.toString() + "; " + indexerEq.toString() + ";";

        ///////// build string
        cb.addCm(_ident->getGlobalName());
        //assert(_asb->checkDesIsFullyAssignAndEqual());
        createOpWithSoleCondition(cb, auxAssVal);
    }


    void MemEleHolderSimEngine::createOpEndCycle(CbBaseCxx& cb){

        if (_master->isWriteMode()){
            cb.addCm(_ident->getGlobalName());

            CbIfCxx&  ifBlock = cb.addIf(getIsSetVar().toString());
            ValR memBlkValR = _master->_master->getSimEngine()->getValRep();

            ValR assEq = memBlkValR.index(getIndexerVar()).eq(getValRep());
            ValR rstSetFlag = getIsSetVar().eq(ValR(SIM_VALREP_TYPE_ALL(1), 1, "0"));

            ///////////// add value
            ifBlock.addSt(assEq.toString());
            ///////////// reset is set
            ifBlock.addSt(rstSetFlag.toString());
        }

    }
}