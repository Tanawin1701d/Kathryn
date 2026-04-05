//
// Created by tanawin on 28/11/2566.
//

#include "expression.h"
#include "model/controller/controller.h"
#include "sim/modelSimEngine/hwComponent/expression/expressionSim.h"

namespace kathryn{

    /**
     * exprMetas
     * */

    expression::expression(LOGIC_OP op,
                           const Operable* a,
                           const Operable* b,
                           int expSize):
    LogicComp<expression>({0, expSize},
                          TYPE_EXPRESSION,
                          new expressionSimEngine(this, VST_WIRE),
                          false),
    _valueAssinged(true),
    _op(op),
    _a(constCast<Operable *>(a)),
    _b(constCast<Operable *>(b))
    {
        comInit();
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }

    expression::expression(int expSize):
    LogicComp<expression>({0, expSize},
                          TYPE_EXPRESSION,
                          new expressionSimEngine(this, VST_WIRE),
                          false),
    _valueAssinged(false),
    _op(ASSIGN),
    _a(nullptr),
    _b(nullptr){
        comInit();
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }


    void expression::comInit() {
        ctrl->onExpressionInit(this);
    }

    void expression::doNonBlockAsm(Operable &srcOpr, Slice desSlice) {
        assert(!_valueAssinged);
        doNonBlockAsmMulAssCheck(srcOpr, desSlice);
    }

    void expression::doNonBlockAsmMulAssCheck(Operable& srcOpr, Slice desSlice){
        mfAssert(getAssignMode() == AM_MOD, "expression can use operator = only in MD mode");
        _a = &srcOpr;
        assert(srcOpr.getOperableSlice().getSize() == getOperableSlice().getSize());
        assert(desSlice.getSize() == getOperableSlice().getSize());
        mfAssert(!_valueAssinged, "multiple expression assign detect");
        _valueAssinged = true;
    }



    SliceAgent<expression>& expression::operator()(int start, int stop) {
        auto ret =  new SliceAgent<expression>(this,
                                               getAbsSubSlice(start, stop, getSlice()));
        return *ret;
    }

    SliceAgent<expression>& expression::operator()(int idx) {
        return operator() (idx, idx+1);
    }

    SliceAgent<expression>& expression::operator()(Slice sl) {
        return operator() (sl.start, sl.stop);
    }

    Operable* expression::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }

    Operable* expression::checkShortCircuit(){
        if (isInCheckPath){
            std::cout << "path end " << std::toString(castToIdent()->getGlobalId()) << std::endl;
            return this;
        }
        isInCheckPath = true;

        Operable* result;
        if (_a != nullptr){
            result = _a->checkShortCircuit();
            if (result != nullptr){
                std::cout << "path a " << std::toString(castToIdent()->getGlobalId()) << std::endl;
                return result;
            }
        }
        if (_b != nullptr){
            result = _b->checkShortCircuit();
            if (result != nullptr){
                std::cout << "path b " << std::toString(castToIdent()->getGlobalId()) << std::endl;
                return result;
            }
        }

        isInCheckPath = false;
        return nullptr;
    }

    void expression::createLogicGen(){
        _genEngine = new ExprGen(
            _parent->getModuleGenPtr(),
            this
        );
    }





}