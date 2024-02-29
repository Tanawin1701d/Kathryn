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



    MemBlockEleHolder& MemBlockEleHolder::operator<<=(Operable &b) {
        mfAssert(getAssignMode() == AM_MOD, "must be Model mode only");
        mfAssert(isReadMode(), "duplicate write operation");
        mfAssert(getSlice().getSize() == b.getOperableSlice().getSize(),
                 "invalid write size");
        setWriteMode();
        assert(b.getOperableSlice().getSize() == _master->getWidthSize());
        ctrl->on_memBlkEleHolder_update(
                generateBasicNode(b,
                                           {0, _master->getWidthSize()}),
                this);
        return *this;
    }

    MemBlockEleHolder& MemBlockEleHolder::operator <<= (ull b){
        mfAssert(getAssignMode() == AM_MOD, "must be Model mode only");
        Operable& rhs = getMatchAssignOperable(b, getSlice().getSize());
        return operator<<=(rhs);
    }

    void MemBlockEleHolder::generateAssMetaForBlocking(Operable& srcOpr,
                                    std::vector<AssignMeta*>& resultMetaCollector,
                                    Slice  absSrcSlice,
                                    Slice  absDesSlice){

        /** for assigning in memblock must have same size */
        mfAssert(isReadMode(), "duplicate write operation");
        mfAssert(getSlice().getSize() == srcOpr.getOperableSlice().getSize(),
                 "invalid write size");
        setWriteMode();
        generateAssignMetaAndFill(srcOpr,resultMetaCollector,
                                  absSrcSlice,absDesSlice);
    }

    MemBlockEleHolder &MemBlockEleHolder::operator=(Operable &b) {
        mfAssert(false, "memBlockEle doesn't support = operator");
        assert(false);
    }

    MemBlockEleHolder &MemBlockEleHolder::operator=(ull b) {
        mfAssert(false, "memBlockEle doesn't support = operator");
        assignSimValue(b);
        return *this;
    }

    void MemBlockEleHolder::generateAssMetaForNonBlocking(Operable& srcOpr,
                                                          std::vector<AssignMeta*>& resultMetaCollector,
                                                          Slice  absSrcSlice,
                                                          Slice  absDesSlice){
        mfAssert(false, "memBlockEle doesn't support generateAssMetaForNonBlocking");
        assert(false);
    }


    /*** override Operable*/
    Slice MemBlockEleHolder::getOperableSlice() const {return getSlice();}
    Operable &MemBlockEleHolder::getExactOperable() const { return (Operable &)(*this);}
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
        auto x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }

    MemBlockEleHolder&
    MemBlockEleHolder::callBackBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        mfAssert(false, "can't assign data to memBlock from slice");
        assert(false);
    }

    MemBlockEleHolder&
    MemBlockEleHolder::callBackNonBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        mfAssert(false, "can't assign data to memBlock from slice");
        assert(false);
    }

    void
    MemBlockEleHolder::callBackBlockAssignFromAgent(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                                    Slice absSrcSlice, Slice absDesSlice) {
        mfAssert(false, "can't assign data to memBlock from slice");
        assert(false);
    }

    void
    MemBlockEleHolder::callBackNonBlockAssignFromAgent(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                                       Slice absSrcSlice, Slice absDesSlice) {
        mfAssert(false, "can't assign data to memBlock from slice");
        assert(false);
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
        ////// then copy the value to destination first incase value is not update
        assert(!isReadMode());
        ////// curAgentVal is assigned already
        assert(isNextValSim() == false);
        setNextValSimStatus();
        /// std::cout << "next======" << std::endl;

        _nextAgentVal = _master->getCurMemVal();
        _master->assignValRepCurCycle(_nextAgentVal);
    }





}