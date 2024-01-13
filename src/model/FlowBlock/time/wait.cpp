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
    FlowBlockCondWait::FlowBlockCondWait(Operable *exitCond)
    : FlowBlockBase(CONDWAIT),
    _resultNodeWrap(nullptr),
    _exitCond(exitCond),
    _waitNode(nullptr)
    {
        assert(exitCond != nullptr);
    }

    FlowBlockCondWait::~FlowBlockCondWait(){
        delete _waitNode;
        delete _resultNodeWrap;
        FlowBlockBase::~FlowBlockBase();
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

        /** build node*/
        _waitNode = new WaitCondNode(_exitCond);
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


    /***
     *
     * cycle wait
     *
     * */



    FlowBlockCycleWait::FlowBlockCycleWait(int cycleUsed)
    : FlowBlockBase(CLKWAIT),
      _resultNodeWrap(nullptr),
      _waitNode(nullptr),
      cycle(-1),
      cnt(nullptr)
    {
        assert(cycle > 0);
    }

    FlowBlockCycleWait::FlowBlockCycleWait(Operable *opr)
    : FlowBlockBase(CLKWAIT),
    _resultNodeWrap(nullptr),
    _waitNode(nullptr),
    cycle(-1),
    cnt(opr)
    {
        assert(opr != nullptr);
    }

    FlowBlockCycleWait::~FlowBlockCycleWait(){
        delete _waitNode;
        delete _resultNodeWrap;
        FlowBlockBase::~FlowBlockBase();
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

        /** build node*/
        if (cnt != nullptr)
            _waitNode = new WaitCycleNode(cnt);
        else
            assert(cycle > 0);
            _waitNode = new WaitCycleNode(cycle);
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


}