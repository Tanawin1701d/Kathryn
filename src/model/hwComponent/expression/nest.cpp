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
                    new NestSimEngine(this, VST_WIRE),
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
        Slice actualSlice = desSlice.getMatchSizeSubSlice(srcOpr.getOperableSlice());
        std::vector<AssignMeta*> resultCollector;
        /** get assign meta to build node*/
        doBlockAsm(srcOpr,
                   resultCollector,
                   srcOpr.getOperableSlice(),
                   actualSlice);
        /** basic node building*/
        auto* asmNode = new AsmNode(resultCollector);
        /** update node*/
        ctrl->on_nest_update(asmNode, this);
    }

    void nest::doNonBlockAsm(Operable &srcOpr, Slice desSlice) {
        assert(desSlice.getSize() <= getSlice().getSize());
        Slice actualSlice = desSlice.getMatchSizeSubSlice(srcOpr.getOperableSlice());
        std::vector<AssignMeta*> resultCollector;
        /** get assign meta to build node*/
        doNonBlockAsm(srcOpr,
                   resultCollector,
                   srcOpr.getOperableSlice(),
                  actualSlice);
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
    nest::doNestGlobalAsm(Operable& srcOpr,
                          std::vector<AssignMeta *> &resultMetaCollector,
                          Slice absSrcSlice,
                          Slice absDesSlice,
                          bool isblockingAsm){

        assert(absSrcSlice.getSize() >= absDesSlice.getSize());
        assert(absSrcSlice.stop <= srcOpr.getOperableSlice().getSize());
        assert(absDesSlice.stop <= getOperableSlice().getSize());

        int desIterAccumBit = 0;
        int srcConsummedBit = 0;

        for (auto desNest: _nestList){
            /** iterate to meet good point first*/
            auto desOpr = desNest.opr;
            auto desAsb= desNest.asb;
            Slice curDesSlice     = desOpr->getOperableSlice();
            int   curDesSize      = curDesSlice.getSize();
            /** if it is not desire destination continue next*/
            if (!absDesSlice.isIntersec({desIterAccumBit, desIterAccumBit + curDesSize})){
                desIterAccumBit += curDesSize;
                continue;
            }

            /** get relative index of current destination*/
            int relDesStartBit = std::max(0         , absDesSlice.start - desIterAccumBit);
            int relDesStopBit  = std::min(curDesSize, absDesSlice.stop  - desIterAccumBit);
            /**convert to abs index of current destination*/
            Slice desireDesSlice = curDesSlice.getSubSlice({relDesStartBit, relDesStopBit});

            /////// calculate src slice
                /**trim start bit*/
            Slice desireSrcSlice = absSrcSlice.getSubSlice(srcConsummedBit);
                /** trim stop bit*/
                  desireSrcSlice = desireSrcSlice.getMatchSizeSubSlice(desireDesSlice);

            if (isblockingAsm){
                desAsb->doBlockAsm(srcOpr,
                                   resultMetaCollector,
                                   desireSrcSlice,
                                   desireDesSlice);
            }else{
                desAsb->doNonBlockAsm(srcOpr,
                                      resultMetaCollector,
                                      desireSrcSlice,
                                      desireDesSlice);
            }

            srcConsummedBit += desireDesSlice.getSize();
            desIterAccumBit += curDesSize;

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

    SliceAgent<nest> &nest::operator()(Slice sl) {
        return operator()(sl.start, sl.stop);
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

    NestSimEngine::NestSimEngine(nest* master,
                             VCD_SIG_TYPE sigType):
            LogicSimEngine(master, master, sigType, false, 0),
            _master(master){
        assert(_master != nullptr);
    }

    void NestSimEngine::proxyBuildInit(){
        for (NestMeta& meta: _master->_nestList){
            dep.push_back(meta.opr->getLogicSimEngineFromOpr());
        }
    }


    std::string NestSimEngine::createOp(){
        ///////// build string
        std::string retStr = "      { /////" + _ident->getGlobalName() + "\n";

        /////////// we build from low priority to high priority

        int startIdx = 0;

        for (NestMeta& meta: _master->_nestList){
            //////// data preparation
            Operable* opr   = meta.opr;
            Assignable* asb = meta.asb;
            assert(opr != nullptr);
            assert(asb != nullptr);
            int curSize = opr->getOperableSlice().getSize();

            retStr += "     ";
            retStr += getVarName() +
                ".updateOnSlice<"+ std::to_string(startIdx) + "," +
                                   std::to_string(startIdx + curSize) + ">(" ;
            retStr += getVarNameFromOpr(opr);
            retStr += ".sliceAndShift<"+std::to_string(opr->getOperableSlice().start) + "," +
                                        std::to_string(opr->getOperableSlice().stop ) + "," +
                                        std::to_string(startIdx)
                                       +">());\n";
            startIdx += curSize;
        }
        retStr += "     }\n";
        assert(startIdx == _asb->getAssignSlice().getSize());
        return retStr;
    }

}