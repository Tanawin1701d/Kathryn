//
// Created by tanawin on 25/2/2567.
//

#include "nest.h"
#include "model/controller/controller.h"
#include "sim/modelSimEngine/hwComponent/expression/nestSim.h"

namespace kathryn{


    /** pre function declaration*/

    std::vector<NestMeta> getNestVec(){
        return {};
    }
    std::vector<Operable*> getNestVecReadOnly(){
        return {};
    }

    nest& makeNestMan(bool isUser, const std::vector<NestMeta>& groupedMeta){
        int nestSize = 0;
        for (NestMeta nestMeta: groupedMeta){
            nestSize += nestMeta.opr->getOperableSlice().getSize();
        }
        return _make<nest>("uncatagorizedYet", "nest", isUser, nestSize, groupedMeta);
    }

    nest& makeNestManReadOnly(bool isUser,
        const std::vector<Operable*>& nestListReadOnly){
        int nestSize = 0;
        for(Operable* opr: nestListReadOnly){
            assert(opr != nullptr);
            nestSize += opr->getOperableSlice().getSize();
        }
        return _make<nest>("uncatagorizedYet", "nest", isUser, nestSize, nestListReadOnly);
    }

    /** nest class*/

    void nest::com_init() {
        ctrl->on_nest_init(this);
    }

    nest::nest(int size, std::vector<NestMeta> nestList):
    LogicComp<nest>({0, size}, TYPE_NEST,
                    new NestSimEngine(this, VST_WIRE),
                    false),
    readOnly(false),
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

    //////////// for read only element

    nest::nest(int size, const std::vector<Operable*>& nestListReadOnly):
    LogicComp<nest>({0, size}, TYPE_NEST,
                    new NestSimEngine(this, VST_WIRE),
                    false),
    readOnly(true)
    {
        int testSize = 0;
        for (Operable* opr: nestListReadOnly){
            assert(opr != nullptr);
            testSize += opr->getOperableSlice().getSize();
            _nestList.push_back({opr, nullptr});
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
        mfAssert(!readOnly, "this nest list is readonly mode");
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

    void nest::createLogicGen(){
        _genEngine = new NestGen(
            _parent->getModuleGen(),
            this
        );
    }

}