//
// Created by tanawin on 30/11/2566.
//

#include "value.h"

#include <utility>
#include "model/controller/controller.h"
#include "util/numberic/numConvert.h"
#include "sim/modelSimEngine/hwComponent/value/valueSim.h"


namespace kathryn{

    /** pad/truncate LSB-first words to ceil(size/64) elements */
    static std::vector<ull> padWideWords(int size, std::vector<ull> words){
        int arrSize = (size + bitSizeOfUll - 1) / bitSizeOfUll;
        words.resize(arrSize, 0);
        return words;
    }

    /** primary constructor — the ull and string forms delegate here */
    Val::Val(int size, std::vector<ull> words):
            LogicComp({0, size},
                      TYPE_VAL,
                      new ValSimEngine(this, VST_INTEGER, words.empty() ? 0 : words[0]),
                      false),
            _size(size),
            _rawValue(words.empty() ? 0 : words[0]),
            _rawValueWide(padWideWords(size, std::move(words)))
    {
        assert(size > 0);
        com_init();
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }

    Val::Val(int size, ull rawValue):
            Val(size, std::vector<ull>{rawValue}) {}

    Val::Val(int size, const std::string& literal):
            Val(size, parseLiteralToWords(literal, size)) {}

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