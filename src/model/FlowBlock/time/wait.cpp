//
// Created by tanawin on 2/1/2567.
//

#include "wait.h"


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
    {}


    NodeWrap *FlowBlockCondWait::sumarizeBlock() {
        assert(_resultNodeWrap != nullptr);
        return _resultNodeWrap;
    }

    void FlowBlockCondWait::onAttachBlock() {
        ////// todo communicate to control flow block
    }

    void FlowBlockCondWait::onDetachBlock() {
        ////// todo communicate to control flow block
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

    /***
     *
     * cycle wait
     *
     * */

}