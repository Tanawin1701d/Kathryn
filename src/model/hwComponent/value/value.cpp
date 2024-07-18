//
// Created by tanawin on 30/11/2566.
//

#include "value.h"

#include <utility>
#include "model/controller/controller.h"
#include "util/numberic/numConvert.h"
#include "sim/modelSimEngine/hwComponent/value/valueSim.h"


namespace kathryn{

    Val::Val(int size, ull rawValue):
            LogicComp({0, size},
                      TYPE_VAL,
                      new ValSimEngine(this,VST_INTEGER, rawValue),
                      false),
            _size(size),
            _rawValue(rawValue)
    {
        assert(size > 0);
        com_init();
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }

    void Val::com_init() {
        ctrl->on_value_init(this);
    }

    SliceAgent<Val>& Val::operator()(int start, int stop){
        auto ret = new SliceAgent<Val>(
                this,
                getAbsSubSlice(start, stop, getSlice())
                );
        return *ret;
    }

    SliceAgent<Val>& Val::operator() (int idx){
        return operator() (idx, idx+1);
    }

    SliceAgent<Val>& Val::operator() (Slice sl){
        return operator() (sl.start, sl.stop);
    }


    Operable* Val::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }


    Operable* Val::checkShortCircuit() {
        return nullptr;
    }

    void Val::createLogicGen(){
        _genEngine = new ValueGen(
            _parent->getModuleGen(),
            this
        );
    }

}