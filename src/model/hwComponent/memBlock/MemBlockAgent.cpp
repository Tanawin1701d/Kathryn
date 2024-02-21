//
// Created by tanawin on 19/2/2567.
//

#include "MemBlockAgent.h"
#include "MemBlock.h"
#include "model/hwComponent/abstract/makeComponent.h"
#include "model/controller/controller.h"

namespace kathryn{

    MemBlockEleHolder::MemBlockEleHolder(MemBlock *master, const Operable* indexer):
    Assignable<MemBlockEleHolder>(),
    Operable(),
    Slicable<MemBlockEleHolder>({0, master->getWidthSize()}),
    Identifiable(TYPE_MEM_BLOCK_INDEXER),
    MemAgentSimulatable(master->getWidthSize()),
    ModelDebuggable(),
    _master(master),
    _indexer(const_cast<Operable*>(indexer)){
        assert(_master  != nullptr);
        assert(_indexer != nullptr);
        assert(_indexer->getOperableSlice().getSize()
               == getExactIndexSize());
    }


    MemBlockEleHolder::MemBlockEleHolder(MemBlock * master, const int idx):
    Assignable<MemBlockEleHolder>(),
    Operable(),
    Slicable<MemBlockEleHolder>({0, master->getWidthSize()}),
    Identifiable(TYPE_MEM_BLOCK_INDEXER),
    MemAgentSimulatable(master->getWidthSize()),
    ModelDebuggable(),
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
        assert(isSetMode());
        /**start simulate it first*/
        _indexer->getSimItf()->simStartCurCycle();
        ValRep& curValIndexer = _indexer->getRtlValItf()->getCurVal();
        assert(curValIndexer.getLen() > 0);
        return _master->getThisCycleValRep(curValIndexer.getVal()[0]);
    }

    int
    MemBlockEleHolder::getExactIndexSize(){
        assert(_master != nullptr);
        return log2Ceil(_master->getDepthSize());
    }



    MemBlockEleHolder& MemBlockEleHolder::operator<<=(Operable &b) {
        mfAssert(!isSetMode(), "duplicate write operation");
        mfAssert(getSlice().getSize() == b.getOperableSlice().getSize(),
                 "invalid write size");
        setWriteMode();
        /**  TODO call back for communication to model controller*/


        return *this;
    }

    MemBlockEleHolder &MemBlockEleHolder::operator=(Operable &b) {
        mfAssert(false, "memBlockEle doesn't support = operator");
    }


    /*** override Operable*/
    Slice MemBlockEleHolder::getOperableSlice() const {return getSlice();}
    Operable &MemBlockEleHolder::getExactOperable() const { return (Operable &)(*this);}
    Identifiable* MemBlockEleHolder::castToIdent() {return this;}

    ValRep &MemBlockEleHolder::sv(){assert(false);}

    /***slicable*/

    SliceAgent<MemBlockEleHolder>&
    MemBlockEleHolder::operator()(int start, int stop) {
        mfAssert(!isSetMode(), "memBlockEle is used");
        setReadMode();
        auto ret = new SliceAgent<MemBlockEleHolder>(this,
                                                     getAbsSubSlice(start, stop,getSlice())
                                                     );
        return *ret;
    }

    SliceAgent<MemBlockEleHolder> &MemBlockEleHolder::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    MemBlockEleHolder&
    MemBlockEleHolder::callBackBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        mfAssert(false, "can't assign data to memBlock from slice");
    }

    MemBlockEleHolder&
    MemBlockEleHolder::callBackNonBlockAssignFromAgent(Operable &b, Slice absSliceOfHost) {
        mfAssert(false, "can't assign data to memBlock from slice");
    }


    /** override rtl simEngine*/

    void MemBlockEleHolder::prepareSim(){
        if (!isSetMode()){
            ////// we so sure that it is read mode
            setReadMode();
        }
    }

    void MemBlockEleHolder::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        _curAgentVal =  &getCurMemVal();
    }

    void MemBlockEleHolder::simStartNextCycle() {
        assert(isSetMode());
        ////// then copy the value to destination first incase value is not update
        if (isNextValSim()) {
            return;
        }
        setNextValSimStatus();

        if (!isReadMode()) {
            ////// curAgentVal is assigned already
            _nextAgentVal = getCurMemVal();
            assignValRepCurCycle(_nextAgentVal);
        }

    }

}