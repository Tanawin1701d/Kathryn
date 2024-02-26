//
// Created by tanawin on 25/2/2567.
//

#include "nest.h"
#include "model/controller/controller.h"

namespace kathryn{


    /** pre function declaration*/

    std::vector<Operable*> getNestVec(){
        return {};
    }

    /** nest class*/

    void nest::com_init() {
        ctrl->on_nest_init(this);
    }

    nest::nest(int size, std::vector<NestMeta> nestList):
    LogicComp<nest>({0, size}, TYPE_NEST,
                    new NestLogicSim(this, size,VST_WIRE, false),
                    false),
    _nestList(std::move(nestList))
    {
        int testSize = 0;
        for (NestMeta meta: _nestList){
            assert(meta.opr != nullptr);
            testSize += meta.opr->getOperableSlice().getSize();
        }
        assert(testSize == size);
        com_init();
    }


    /** assign enforcer*/


    void nest::generateAssMetaForBlocking(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                          Slice absSrcSlice, Slice absDesSlice) {
        generateAssMetaForAll(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice, true);
    }

    void nest::generateAssMetaForNonBlocking(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                             Slice absSrcSlice, Slice absDesSlice) {
        generateAssMetaForAll(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice, false);
    }


    void
    nest::generateAssMetaForAll(Operable &srcOpr,
                                std::vector<AssignMeta *> &resultMetaCollector,
                                Slice absSrcSlice,
                                Slice absDesSlice,
                                bool isBlockingAsm){

        Slice curSlice = absSrcSlice;
        for (NestMeta meta: _nestList){
            Operable* subOpr   = meta.opr;
            Assignable* subAss = meta.asb;
            assert(subOpr != nullptr);
            if (curSlice.start >= curSlice.stop){
                break;
            }
            if (isBlockingAsm){
                subAss->generateAssMetaForBlocking(
                        srcOpr,
                        resultMetaCollector,
                        curSlice,
                        subOpr->getOperableSlice()
                );
            }else{
                subAss->generateAssMetaForNonBlocking(
                        srcOpr,
                        resultMetaCollector,
                        curSlice,
                        subOpr->getOperableSlice()
                );
            }

            curSlice.start += curSlice.getMatchSizeSubSlice(subOpr->getOperableSlice()).getSize();
        }

    }
    /**********************************/

    SliceAgent<nest> &nest::operator()(int start, int stop) {
        auto ret =  new SliceAgent<nest>(this,getAbsSubSlice(start, stop, getSlice()));
        return *ret;
    }

    SliceAgent<nest> &nest::operator()(int idx) {
        return operator()(idx, idx + 1);
    }

    nest& nest::callBackBlockAssignFromAgent(Operable &b, Slice absSlice) {
        ///// TODO make it assignable
        assert(false);
    }

    nest& nest::callBackNonBlockAssignFromAgent(Operable &b, Slice absSlice) {
        ///// TODO make it assignable
        assert(false);
    }

    void nest::callBackBlockAssignFromAgent(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                            Slice absSrcSlice, Slice absDesSlice) {
        assert(getSlice().isContain(absDesSlice));
        generateAssMetaForBlocking(srcOpr,
                                   resultMetaCollector,
                                   absSrcSlice,
                                   absDesSlice);
    }

    void nest::callBackNonBlockAssignFromAgent(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                                               Slice absSrcSlice, Slice absDesSlice) {
        assert(getSlice().isContain(absDesSlice));
        generateAssMetaForNonBlocking(srcOpr,
                                    resultMetaCollector,
                                       absSrcSlice,
                                       absDesSlice);
    }




    Operable* nest::checkShortCircuit() {
        if (isInCheckPath){
            return this;
        }

        isInCheckPath = true;

        for (NestMeta meta: _nestList){

            Operable* result = meta.opr->checkShortCircuit();
            if (result != nullptr){
                return result;
            }
        }

        isInCheckPath = false;
        return nullptr;
    }



    /***
     *
     * NestLogicSim
     *
     * */

    NestLogicSim::NestLogicSim(nest* master,
                             int sz,
                             VCD_SIG_TYPE sigType,
                             bool simForNext):
            LogicSimEngine(sz, sigType, simForNext),
            _master(master){}

    void NestLogicSim::simStartCurCycle() {
        if (isCurValSim()){
            return;
        }
        setCurValSimStatus();

        /**get sim value and reset it*/
        ValRep& desValRep = getCurVal();
        desValRep.fillZeroToValrep(0);

        int curStartBit = 0;

        for (NestMeta meta: _master->_nestList){
            Operable* opr = meta.opr;
            /**slave opr first*/
            assert(opr != nullptr);
            opr->getSimItf()->simStartCurCycle();
            assert(opr->getRtlValItf()->isCurValSim());
            ValRep oprVal = opr->getSlicedCurValue();
            assert(oprVal.getLen() == opr->getOperableSlice().getSize());

            ///std::cout << oprVal.getBiStr() << std::endl;

            /** cal next opr*/
            desValRep.updateOnSlice(oprVal,
                                    {curStartBit,
                                           curStartBit + opr->getOperableSlice().getSize()}
                                           );
            curStartBit += opr->getOperableSlice().getSize();

        }
        assert(curStartBit == _master->getSlice().getSize());


    }




}