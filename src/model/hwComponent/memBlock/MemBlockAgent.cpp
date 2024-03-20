//
// Created by tanawin on 19/2/2567.
//

#include "MemBlockAgent.h"
#include "MemBlock.h"
#include "model/hwComponent/abstract/makeComponent.h"
#include "model/controller/controller.h"

namespace kathryn{

    MemBlockEleHolder::MemBlockEleHolder(MemBlock *master, const Operable* indexer):
            LogicComp<MemBlockEleHolder>({0, master->getWidthSize()},
                                         TYPE_MEM_BLOCK_INDEXER,
                                         new MemEleHolderLogicSim(this, master->getWidthSize()),
                                         false
                                          ),
            readMode(true),
            _master(master),
            _indexer(const_cast<Operable*>(indexer)){
        assert(_master  != nullptr);
        assert(_indexer != nullptr);
        assert(_indexer->getOperableSlice().getSize()
               == getExactIndexSize());
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }


    MemBlockEleHolder::MemBlockEleHolder(MemBlock * master, const int idx):
            LogicComp<MemBlockEleHolder>({0, master->getWidthSize()},
                                         TYPE_MEM_BLOCK_INDEXER,
                                         new MemEleHolderLogicSim(this, master->getWidthSize()),
                                         false
            ),
            _master(master),
            _indexer(nullptr)
    {
        makeVal(memIndexer, getExactIndexSize(), idx);
        _indexer = (Operable*)(&memIndexer);
        assert(_master != nullptr);
        assert(_indexer!= nullptr);
    }

    ValRep&
    MemBlockEleHolder::getCurMemVal(){
        /**start simulate it first*/
        _indexer->getSimItf()->simStartCurCycle();
        ValRep curValIndexer = _indexer->getSlicedCurValue();
        assert(curValIndexer.getLen() == getExactIndexSize());
        /////std::cout << "indexer ==== " << curValIndexer.getVal()[0] << std::endl;
        ////TODO we will deal overflow index later with this later

        ull curIdx =  curValIndexer.getVal()[0];

        if (curIdx >= _master->getDepthSize()){
            curIdx  = 0;
        }

        return _master->getThisCycleValRep(curIdx);
    }

    int
    MemBlockEleHolder::getExactIndexSize(){
        assert(_master != nullptr);
        return log2Ceil(_master->getDepthSize());
    }



    void MemBlockEleHolder::doBlockAsm(Operable &srcOpr, Slice desSlice) {
        mfAssert(getAssignMode() == AM_MOD, "must be Model mode only");
        mfAssert(isReadMode(), "duplicate write operation");
        mfAssert(getSlice().getSize() == srcOpr.getOperableSlice().getSize(),
                 "invalid write size");
        setWriteMode();
        assert(srcOpr.getOperableSlice().getSize() == _master->getWidthSize());
        ctrl->on_memBlkEleHolder_update(
                generateBasicNode(srcOpr,
                 {0, _master->getWidthSize()}),
                this);
    }

    void MemBlockEleHolder::doNonBlockAsm(Operable& srcOpr, Slice desSlice) {
        mfAssert(false, "memBlockEle doesn't support = operator");
        assert(false);
    }

    void MemBlockEleHolder::doBlockAsm(Operable& srcOpr,
                                       std::vector<AssignMeta*>& resultMetaCollector,
                                       Slice  absSrcSlice,
                                       Slice  absDesSlice){

        /** for assigning in memblock must have same size */
        mfAssert(isReadMode(), "duplicate write operation");
        mfAssert(getSlice().getSize() == srcOpr.getOperableSlice().getSize(),
                 "invalid write size");
        setWriteMode();
        doGlobalAsm(srcOpr, resultMetaCollector,
                    absSrcSlice, absDesSlice);
    }

    void MemBlockEleHolder::doNonBlockAsm(Operable& srcOpr,
                                          std::vector<AssignMeta*>& resultMetaCollector,
                                          Slice  absSrcSlice,
                                          Slice  absDesSlice){
        mfAssert(false, "memBlockEle doesn't support doNonBlockAsm");
        assert(false);
    }

    /*** override Operable*/
    Identifiable* MemBlockEleHolder::castToIdent() {return this;}

    ValRep &MemBlockEleHolder::sv(){assert(false);}

    /***slicable*/

    SliceAgent<MemBlockEleHolder>&
    MemBlockEleHolder::operator()(int start, int stop) {
        mfAssert(!isReadMode(), "memBlockEle is used");
        setReadMode();
        auto ret = new SliceAgent<MemBlockEleHolder>(this,
                                                     getAbsSubSlice(start, stop,getSlice())
                                                     );
        return *ret;
    }

    SliceAgent<MemBlockEleHolder> &MemBlockEleHolder::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    Operable* MemBlockEleHolder::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }

    /**
     *
     * MemEleSimEngine
     *
     * */

    MemEleHolderLogicSim::MemEleHolderLogicSim(MemBlockEleHolder* master,
                                               int bitWidth):
                          MemAgentSimEngine(bitWidth),
                          _master(master){}

    void MemEleHolderLogicSim::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        ////std::cout << "next======" << std::endl;
        setCurValSimStatus();
        _curAgentVal =  &_master->getCurMemVal();
    }

    void MemEleHolderLogicSim::simStartNextCycle() {
        assert(!isReadMode());
        ////// curAgentVal is assigned already
        assert(isNextValSim() == false);
        setNextValSimStatus();
        /// std::cout << "next======" << std::endl;

        /////// copy current value to _nextAgentVal first then assign next value
        ////////// fyi getCurmemval is pointer to origin value
        _nextAgentVal = _master->getCurMemVal();
        _master->assignValRepCurCycle(_nextAgentVal);
    }





}