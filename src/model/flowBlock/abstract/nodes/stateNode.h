//
// Created by tanawin on 4/1/2567.
//

#ifndef KATHRYN_STATENODE_H
#define KATHRYN_STATENODE_H


#include "node.h"
#include "asmNode.h"

namespace kathryn{


    /**
     * state that represent status of each register in the circuit
     * */

    struct StateNode : Node{
        StateReg* _stateReg;
        std::vector<AsmNode*> _dependSlaveAsmNode; /// the asignment node that depend on this stateNode


        explicit StateNode() :
            Node(STATE_NODE),
            _stateReg(new StateReg()){}




        void addSlaveAsmNode(AsmNode* asmNode){
            assert(asmNode != nullptr);
            asmNode->addDependNode(this);
            _dependSlaveAsmNode.push_back(asmNode);
        }

        void finalize() override{
            _stateReg->setVarName(identName);
            /*** set event*/
            _stateReg->makeSetEvent(dep[CON_NODE_SET]);
            /** unset event*/
            _stateReg->makeUnsetEvent();
            /** reset Interrupt*/
            _stateReg->makeResetIntrSeq(dep[CON_NODE_RESET_INTR]);
            /** start Interrupt*/
            _stateReg->makeStartIntrSeq(dep[CON_NODE_START_INTR]);
            /** slave event*/
            for (AsmNode* asmNode: _dependSlaveAsmNode){
                asmNode->finalizeFromStateNode();
            }
        }

        Operable* getExitOpr() override{
            assert(_stateReg != nullptr);
            return _stateReg->generateEndExpr();
        }

        int getCycleUsed() override {return 1;}

        bool isStateFullNode() override{
            return true;
        }

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_stateReg != nullptr);
            bool isStateSet = _stateReg->getRtlValItf()->getCurVal().getLogicalValue();
            if (isStateSet){
                setBlockOrNodeRunning();
                incEngine();
            }

        }
    };

    /**
         * node that represent syn status of each register in the circuit
         * usually used in parallel block with unknown exact cycle
         * */

    struct SynNode : Node{
        SyncReg* _synReg;

        /**in SynNode condition and dependState is disengage*/
        explicit SynNode(int synSize) :
            Node(SYN_NODE),
            _synReg(new SyncReg(synSize)){}



        void finalize() override{
            _synReg->setVarName(identName);

            /*** set event*/
            _synReg->makeSetEvent(dep[CON_NODE_SET]);
            /** unset event*/
            _synReg->makeUnsetEvent();
            /** reset Interrupt*/
            _synReg->makeResetIntrSeq(dep[CON_NODE_RESET_INTR]);
            /** start Interrupt*/
            _synReg->makeStartIntrSeq(dep[CON_NODE_START_INTR]);
        }

        Operable* getExitOpr() override{return _synReg->generateState();}

        int getCycleUsed() override{ return 0; }

        bool isStateFullNode() override { return false;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_synReg != nullptr);
            bool isFullSyncCycle = _synReg->isSimAtFullSyn();
            /** inc engine*/
            if (isFullSyncCycle){
                setBlockOrNodeRunning();
                incEngine();
            }

        }

    };

    struct PseudoNode : Node{
        Operable* outputExpr = nullptr;

        explicit PseudoNode(int expr_size) :
            Node(PSEUDO_NODE),
            outputExpr(nullptr){}

        void finalize() override{

            /** for pseudoNode  only conNode set is set */

            for (NODE_META& x: dep[CON_NODE_SET]){
                assert(x._state != nullptr);

                if (x._condition == nullptr){
                    addLogic(outputExpr, x._state->getExitOpr(), BITWISE_OR);
                }else{
                    addLogic(outputExpr,
                             &((*x._state->getExitOpr()) & (*x._condition)),
                             BITWISE_OR);
                }
            }
        }

        int getCycleUsed() override { return 0; }

        Operable* getExitOpr() override{assert(outputExpr != nullptr); return outputExpr;}

        bool isStateFullNode() override{ return false; }

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(outputExpr != nullptr);
            /** inc engine do not increase engine*/
        }



    };

    struct DummyNode : Node{
        Val* _value = nullptr;

        explicit DummyNode(Val* value) :
            Node(DUMMY_NODE),
            _value(value){
            assert(_value != nullptr);
        }

        void finalize() override{
            /** we don't support assign from condition or depend state*/
            assert(checkAllDepEmpty());
            _value->setVarName(identName);
        }

        Operable* getExitOpr() override{return _value;}

        int getCycleUsed() override{ return 0; }

        bool isStateFullNode() override{return false;}

        void simStartCurCycle() override{
            if (isCurValSim()){
                return;
            }
            setCurValSimStatus();
            assert(_value != nullptr);
            /** inc engine do not increase engine*/
        }

    };

}


#endif //KATHRYN_STATENODE_H
