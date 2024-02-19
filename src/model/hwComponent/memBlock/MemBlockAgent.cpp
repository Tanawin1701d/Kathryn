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
    RtlSimulatable(new RtlSimEngine(getSlice().getSize(), VST_DUMMY, false)),
    _master(master),
    _indexer(const_cast<Operable *>(indexer)){
        assert(_master  != nullptr);
        assert(_indexer != nullptr);
    }


    MemBlockEleHolder::MemBlockEleHolder(MemBlock * master, const int idx):
    Assignable<MemBlockEleHolder>(),
    Operable(),
    Slicable<MemBlockEleHolder>({0, master->getWidthSize()}),
    Identifiable(TYPE_MEM_BLOCK_INDEXER),
    RtlSimulatable(new RtlSimEngine(getSlice().getSize(), VST_DUMMY, false)),
    _master(master),
    _indexer(nullptr)
    {
        /** todo make right size*/
        makeVal(memIndexer, 7, idx);
        _indexer = (Operable*)(&memIndexer);
        assert(_master != nullptr);
        assert(_indexer!= nullptr);
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

    Slice MemBlockEleHolder::getOperableSlice() const {
        return getSlice();
    }
    Operable &MemBlockEleHolder::getExactOperable() const {
        return (Operable &)(*this);
    }
    ValRep& MemBlockEleHolder::getExactSimCurValue() {
        /***/
        return getSimEngine()->getCurVal();
    }
    ValRep& MemBlockEleHolder::getExactSimNextValue() {
        return getSimEngine()->getNextVal();
    }
    Identifiable* MemBlockEleHolder::castToIdent() {
        return this;
    }
    RtlSimulatable* MemBlockEleHolder::castToRtlSimItf() {
        return this;
    }
    ValRep &MemBlockEleHolder::sv() {
        assert(false);
    }

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
        assert(getSimEngine() != nullptr);
        getSimEngine()->setSimForNext(!isReadMode); /**we sim for next only for write mode*/
    }

    void MemBlockEleHolder::simStartCurCycle() {
        /////// todo callback to master to request
        assert(isSetMode());
        if (isReadMode){
            getSimEngine()->setCurValSimStatus();
            auto& desValue = getSimEngine()->getCurVal();
            assignValRepCurCycle(desValue);
        }
    }

    void MemBlockEleHolder::simStartNextCycle() {
        ////// todo callback to master to request
        assert(isSetMode());
        if (!isReadMode){
            RtlSimEngine* simEngine = getSimEngine();
            assert(simEngine->isCurValSim());
            assert(!simEngine->isNextValSim());

            simEngine->setNextValSimStatus();
            simEngine->getNextVal() = simEngine->getCurVal();
            assignValRepCurCycle(getSimEngine()->getNextVal());
        }
    }

    void MemBlockEleHolder::simExitCurCycle() {
        RtlSimulatable::simExitCurCycle();
    }


}