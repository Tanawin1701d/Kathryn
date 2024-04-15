//
// Created by tanawin on 2/1/2567.
//

#include "wait.h"
#include "model/controller/controller.h"



namespace kathryn{


    /***
     *
     * condition wait
     *
     * */

    /**constructor*/
    FlowBlockCondWait::FlowBlockCondWait(Operable& exitCond)
    : FlowBlockBase(CONDWAIT,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }),
    _resultNodeWrap(nullptr),
    _exitCond(&exitCond),
    _purifiedExitCond(purifyCondition(&exitCond)),
    _waitNode(nullptr)
    {
        //assert(exitCond != nullptr);
    }

    FlowBlockCondWait::~FlowBlockCondWait(){
        delete _waitNode;
        delete _resultNodeWrap;
    }

    NodeWrap* FlowBlockCondWait::sumarizeBlock() {
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockCondWait::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockCondWait::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }
    /** buildHwComponent*/
    void FlowBlockCondWait::buildHwComponent() {
        assert(_conBlocks.empty());
        assert(!isThereIntStart());

        /** build node*/

        _waitNode = new WaitCondNode(_exitCond);
        fillIntResetToNodeIfThere(_waitNode);
        _waitNode->setInternalIdent("waitCond"+std::to_string(getGlobalId()));
        /** result node wrap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->addEntraceNode(_waitNode);
        _resultNodeWrap->addExitNode(_waitNode);

    }

    void FlowBlockCondWait::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockCondWait::doPostFunction() {
        onDetachBlock();
    }

    std::string FlowBlockCondWait::getMdDescribe(){
        std::string ret;
        ret += "[ " + getMdIdentVal() + " ]\n";
        ret += "exitCond is " + _exitCond->castToIdent()->getIdentDebugValue() + "\n";
        ret += "_waitNode is " +_waitNode->getMdIdentVal()+ " " +_waitNode->getMdDescribe() + "\n";
        return ret;
    }

    void FlowBlockCondWait::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + "]");
        mdLogVal->addVal("exitVal cond is " + _exitCond->castToIdent()->getIdentDebugValue());
        mdLogVal->addVal("waitNode is " + _waitNode->getMdIdentVal()+ " " +_waitNode->getMdDescribe());
    }

    void FlowBlockCondWait::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isStateRunning = false;
        /** simulate each element*/
        if (_waitNode != nullptr){
            _waitNode->simStartCurCycle();
            isStateRunning |= _waitNode->isBlockOrNodeRunning();
        }
        /** inc engine*/
        if (isStateRunning) {
            setBlockOrNodeRunning();
            incEngine();
        }
    }

    void FlowBlockCondWait::simExitCurCycle() {
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        if (_waitNode != nullptr){
            _waitNode->simExitCurCycle();
        }

    }


    /***
     *
     * cycle wait
     *
     * */



    FlowBlockCycleWait::FlowBlockCycleWait(int cycleUsed)
    : FlowBlockBase(CLKWAIT,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }),
      _resultNodeWrap(nullptr),
      _waitNode(nullptr),
      cycle(cycleUsed),
      cnt(nullptr)
    {
        assert(cycle > 0);
    }

    FlowBlockCycleWait::FlowBlockCycleWait(Operable& opr)
    : FlowBlockBase(CLKWAIT,
                    {
                            {FLOW_ST_BASE_STACK},
                            FLOW_JO_SUB_FLOW,
                            true
                    }),
    _resultNodeWrap(nullptr),
    _waitNode(nullptr),
    cycle(-1),
    cnt(&opr)
    {
        ///////assert(opr != nullptr);
    }

    FlowBlockCycleWait::~FlowBlockCycleWait(){
        delete _waitNode;
        delete _resultNodeWrap;
    }

    NodeWrap* FlowBlockCycleWait::sumarizeBlock() {
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockCycleWait::onAttachBlock() {
        ctrl->on_attach_flowBlock(this);
    }

    void FlowBlockCycleWait::onDetachBlock() {
        ctrl->on_detach_flowBlock(this);
    }

    void FlowBlockCycleWait::buildHwComponent() {
        assert(_conBlocks.empty());
        assert(!isThereIntStart());

        /** build node*/
        if (cnt != nullptr) {
            _waitNode = new WaitCycleNode(cnt);
        }else {
            assert(cycle > 0);
            _waitNode = new WaitCycleNode(cycle);
        }
        fillIntResetToNodeIfThere(_waitNode);
        _waitNode->setInternalIdent("waitCycle"+std::to_string(getGlobalId()));
        /** result node wrap*/
        _resultNodeWrap = new NodeWrap();
        _resultNodeWrap->addEntraceNode(_waitNode);
        _resultNodeWrap->addExitNode(_waitNode);

    }

    void FlowBlockCycleWait::doPreFunction() {
        onAttachBlock();
    }

    void FlowBlockCycleWait::doPostFunction() {
        onDetachBlock();
    }

    std::string FlowBlockCycleWait::getMdDescribe(){
        std::string ret;
        ret += "\n";
        ret += "[_waitNode] is " + _waitNode->getMdIdentVal() + "  " + _waitNode->getMdDescribe() + "\n";
        ret += "[cnt] is " + cnt->castToIdent()->getIdentDebugValue()+ "\n";
        return ret;
    }

    void FlowBlockCycleWait::addMdLog(MdLogVal *mdLogVal) {
        mdLogVal->addVal("[ " + FlowBlockBase::getMdIdentVal() + "]");
        mdLogVal->addVal( "wait Node is " + _waitNode->getMdIdentVal() + "  " + _waitNode->getMdDescribe());
        mdLogVal->addVal("counter" + cnt->castToIdent()->getIdentDebugValue());
    }

    void FlowBlockCycleWait::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();
        bool isStateRunning = false;
        /** simulate each element*/
        if (_waitNode != nullptr){
            _waitNode->simStartCurCycle();
            isStateRunning |= _waitNode->isBlockOrNodeRunning();
        }
        /** inc engine*/
        if (isStateRunning){
            setBlockOrNodeRunning();
            incEngine();
        }

    }

    void FlowBlockCycleWait::simExitCurCycle() {
        unSetSimStatus();
        unsetBlockOrNodeRunning();
        if (_waitNode != nullptr){
            _waitNode->simExitCurCycle();
        }
    }

}