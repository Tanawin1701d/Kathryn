//
// Created by tanawin on 25/2/2567.
//

#include "nest.h"
#include "model/controller/controller.h"

namespace kathryn{


    /** pre function declaration*/

    std::vector<NestMeta> getNestVec(){
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
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }


    /**
     *
     * blocking assignment
     *
     * */


    void nest::doBlockAsm(Operable& srcOpr, Slice desSlice) {
        assert(desSlice.getSize() <= getSlice().getSize());
        std::vector<AssignMeta*> resultCollector;
        /** get assign meta to build node*/
        doBlockAsm(srcOpr,
                   resultCollector,
                   srcOpr.getOperableSlice(),
                   desSlice);
        /** basic node building*/
        auto* asmNode = new AsmNode(resultCollector);
        /** update node*/
        ctrl->on_nest_update(asmNode, this);
    }

    void nest::doNonBlockAsm(Operable &srcOpr, Slice desSlice) {
        assert(desSlice.getSize() <= getSlice().getSize());
        std::vector<AssignMeta*> resultCollector;
        /** get assign meta to build node*/
        doNonBlockAsm(srcOpr,
                   resultCollector,
                   srcOpr.getOperableSlice(),
                   desSlice);
        /** basic node building*/
        auto* asmNode = new AsmNode(resultCollector);
        /** update node*/
        ctrl->on_nest_update(asmNode, this);
    }

    /** assign enforcer*/


    void nest::doBlockAsm(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                          Slice absSrcSlice, Slice absDesSlice) {
        doNestGlobalAsm(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice, true);
    }

    void nest::doNonBlockAsm(Operable &srcOpr, std::vector<AssignMeta *> &resultMetaCollector,
                             Slice absSrcSlice, Slice absDesSlice) {
        doNestGlobalAsm(srcOpr, resultMetaCollector, absSrcSlice, absDesSlice, false);
    }

    int nest::getNetListIdxThatMatch(int bitIdx) {
        assert(bitIdx > 0);
        int   startAcc = 0;
        for (int i = 0; i < _nestList.size(); i++){
            Operable* subOpr = _nestList[i].opr;
            assert(subOpr != nullptr);
            /****/
            int subOprSize = subOpr->getOperableSlice().getSize();
            if ( (startAcc + subOprSize) < bitIdx){
                startAcc += subOprSize;
            }else{
                return i;
            }
        }
        assert(false);
    }


    void
    nest::doNestGlobalAsm(Operable &srcOpr,
                          std::vector<AssignMeta *> &resultMetaCollector,
                          Slice absSrcSlice,
                          Slice absDesSlice,
                          bool isblockingAsm){

        assert(absDesSlice.checkValidSlice());
        assert(absSrcSlice.checkValidSlice());

        Slice curSrcSlice   = absSrcSlice;
        int accumStart = 0;
        ///std::cout << "------------------------------------" << std::endl;
        /** traverse to all nest list to assign balule*/
        for (NestMeta meta: _nestList){

            /** get neccessary data*/
            Operable* subOpr   = meta.opr;
            Assignable* subAss = meta.asb;
            assert(subOpr != nullptr);
            Slice subOprSlice = subOpr->getOperableSlice();
            int subOprSize = subOpr->getOperableSlice().getSize();
            /** check is src ok*/
            ///std::cout  <<subOpr->getOperableSlice().getSize() << std::endl;
            if (curSrcSlice.start >= curSrcSlice.stop){
                break;
            }

            /** check that current iteration reach specific point*/
            if (!absDesSlice.isIntersec(
                    {accumStart, accumStart + subOprSize})){
                accumStart += subOprSize;
                continue;
            }
            ////std::cout << "trap"  <<subOpr->getOperableSlice().getSize() << std::endl;
            /**get which bit in each element will be used to fill data*/
            int startBit = std::max(0               , absDesSlice.start - accumStart);
            int stopBit  = std::min(subOprSlice.stop, absDesSlice.stop  - accumStart);
            Slice desireDesSlice = {startBit, stopBit};
            ///std::cout << "startBit " <<startBit << "   stopBit " << stopBit << std::endl;

            ///std::cout << "inner" << std::endl;
            if (isblockingAsm){
                subAss->doBlockAsm(
                        srcOpr,
                        resultMetaCollector,
                        curSrcSlice,
                        desireDesSlice
                );
            }else{
                subAss->doNonBlockAsm(
                        srcOpr,
                        resultMetaCollector,
                        curSrcSlice,
                        desireDesSlice
                );
            }
            ////std::cout << "oouter" << std::endl;
            /** iterate srcSlice to getnext*/
            curSrcSlice.start += desireDesSlice.getSize();
            /** iterate accumValue*/
            accumStart += subOprSize;
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

    Operable* nest::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
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