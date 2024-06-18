//
// Created by tanawin on 30/11/2566.
//

#include "value.h"

#include <utility>
#include "model/controller/controller.h"
#include "util/numberic/numConvert.h"


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



    /**
     * value simulation
     * */

    ValSimEngine::ValSimEngine(Val* master,
                             VCD_SIG_TYPE sigType,
                             ull rawValue):
    LogicSimEngine(master, master,
                   VST_INTEGER, false, rawValue),
    _master(master){ assert(master != nullptr);}

    std::string
    ValSimEngine::genSliceTo(Slice desSlice){
        ull _mask = createMask(desSlice);
        ull _val  = _initVal >> desSlice.start;
        return std::to_string(_mask & _val);
    }

    std::string
    ValSimEngine::genSliceToWithFixSize(Slice desSlice, int fixLength){
        return genSliceTo(desSlice);
    }

    std::string
    ValSimEngine::genSliceAndShift(Slice desSlice, Slice srcSlice){
        ull _mask = createMask(srcSlice);
        ull _val  = _initVal >> srcSlice.start;
            _val  = _val & _mask;
        assert(desSlice.start < bitSizeOfUll);
        return std::to_string(_val << desSlice.start);
    }

    std::string
    ValSimEngine::createGlobalVariable(){
        return "const ull " + getVarName() +
        " = " + std::to_string(_initVal) + ";";
    }

}