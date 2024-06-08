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
                                         new MemEleHolderSimEngine(this),
                                         false
                                          ),
            readMode(true),
            _master(master),
            _indexer(const_cast<Operable*>(indexer)){
        assert(_master  != nullptr);
        assert(_indexer != nullptr);
        ////std::cout << _indexer->getOperableSlice().getSize() << "    actual " << getExactIndexSize() << std::endl;
        assert(_indexer->getOperableSlice().getSize()
               == getExactIndexSize());
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }


    MemBlockEleHolder::MemBlockEleHolder(MemBlock * master, const int idx):
            LogicComp<MemBlockEleHolder>({0, master->getWidthSize()},
                                         TYPE_MEM_BLOCK_INDEXER,
                                         new MemEleHolderSimEngine(this),
                                         false
            ),
            readMode(true),
            _master(master),
            _indexer(nullptr)
    {
        makeVal(memIndexer, getExactIndexSize(), idx);
        _indexer = (Operable*)(&memIndexer);
        assert(_master != nullptr);
        assert(_indexer!= nullptr);

        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }

    int
    MemBlockEleHolder::getExactIndexSize(){
        assert(_master != nullptr);
        return log2Ceil(_master->getDepthSize());
    }


    /**
     * standard assignment
     *
     * ***/


    void MemBlockEleHolder::doBlockAsm(Operable &srcOpr, Slice desSlice) {
        doGlobalAsm(srcOpr, desSlice, ASM_DIRECT);
    }

    void MemBlockEleHolder::doNonBlockAsm(Operable& srcOpr, Slice desSlice) {
        doGlobalAsm(srcOpr, desSlice, ASM_EQ_DEPNODE);
    }

    void MemBlockEleHolder::doGlobalAsm(Operable& srcOpr, Slice desSlice, ASM_TYPE asmType){
        mfAssert(getAssignMode() == AM_MOD, "must be Model mode only");
        mfAssert(isReadMode(), "duplicate write operation");
        mfAssert(getSlice().getSize() == srcOpr.getOperableSlice().getSize(),
                 "invalid write size");
        setWriteMode();
        assert(srcOpr.getOperableSlice().getSize() == _master->getWidthSize());
        ctrl->on_memBlkEleHolder_update(
                generateBasicNode(srcOpr,
                                  {0, _master->getWidthSize()},
                                          asmType),
                this);
    }

    /**
     * special group assignment
     *
     * ***/

    void MemBlockEleHolder::doBlockAsm(Operable& srcOpr,
                                       std::vector<AssignMeta*>& resultMetaCollector,
                                       Slice  absSrcSlice,
                                       Slice  absDesSlice){
        /** for assigning in memblock must have same size */
        mfAssert(isReadMode(), "duplicate write operation");
        mfAssert(getSlice().getSize() == absSrcSlice.getSize(),
                 "invalid write size");
        setWriteMode();
        Assignable::doGlobalAsm(srcOpr, resultMetaCollector,
                                   absSrcSlice, absDesSlice,
                                   ASM_DIRECT);
    }

    void MemBlockEleHolder::doNonBlockAsm(Operable& srcOpr,
                                          std::vector<AssignMeta*>& resultMetaCollector,
                                          Slice  absSrcSlice,
                                          Slice  absDesSlice){
        /** for assigning in memblock must have same size */
        mfAssert(isReadMode(), "duplicate write operation");
        mfAssert(getSlice().getSize() == absSrcSlice.getSize(),
                 "invalid write size");
        setWriteMode();
        Assignable::doGlobalAsm(srcOpr, resultMetaCollector,
                                   absSrcSlice, absDesSlice,
                                   ASM_EQ_DEPNODE);
    }

    /*** override Operable*/
    Identifiable* MemBlockEleHolder::castToIdent() {return this;}

    /***slicable*/

    SliceAgent<MemBlockEleHolder>&
    MemBlockEleHolder::operator()(int start, int stop) {
        mfAssert(isReadMode(), "mem holder is used for write mode");
        setReadMode();
        auto ret = new SliceAgent<MemBlockEleHolder>(this,
                                                     getAbsSubSlice(start, stop,getSlice())
                                                     );
        return *ret;
    }

    SliceAgent<MemBlockEleHolder> &MemBlockEleHolder::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    SliceAgent<MemBlockEleHolder> &MemBlockEleHolder::operator()(Slice sl) {
        return operator() (sl.start, sl.stop);
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

    MemEleHolderSimEngine::MemEleHolderSimEngine(MemBlockEleHolder* master):
    LogicSimEngine(master, master, VST_DUMMY, false, 0),
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

    std::string MemEleHolderSimEngine::createOp(){
        if (_master->isWriteMode()){ ///// write mode
            return "";
        }
        ///
        /////// read mode
        ///
        std::string retStr = "{ ///////" + _ident->getGlobalName() + "readMode\n";

        retStr += "     ";
        retStr += getVarName() + " = ";
        retStr += _master->_master->getSimEngine()->getVarName();
        retStr += "[" + getVarNameFromOpr(_master->_indexer) + "];\n";
        return retStr;
    }

    std::string MemEleHolderSimEngine::createOpEndCycle(){
        if (_master->isReadMode()){
            return "";
        }

        ///////// build string
        std::string retStr = "      { /////" + _ident->getGlobalName() + "\n";

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
            retStr += _master->_master->getSimEngine()->getVarName();
            retStr += "[" + getVarNameFromOpr(_master->_indexer) + "] = ";
            retStr += getVarNameFromOpr(updateEvent->srcUpdateValue);
            retStr += ".slice<"  + std::to_string(updateEvent->srcUpdateValue->getOperableSlice().start) + "," +
                                   std::to_string(updateEvent->srcUpdateValue->getOperableSlice().stop ) + ">();\n";
            retStr += "         }\n";
        }

        retStr += "     }\n";
        return retStr;
    }







}