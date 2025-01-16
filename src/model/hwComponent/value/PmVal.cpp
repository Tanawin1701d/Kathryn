//
// Created by tanawin on 14/1/2025.
//

#include "PmVal.h"
#include <utility>
#include "model/controller/controller.h"
#include "sim/modelSimEngine/hwComponent/value/pmValueSim.h"
#include "gen/proxyHwComp/value/paramValueGen.h"


namespace kathryn{


    PmVal::PmVal(ull rawValue):
        LogicComp({0, _size},
            TYPE_PMVAL,
            new PmValSimEngine(this, VST_INTEGER),
            false),
        _size(64),
        _rawValue(rawValue)
    {
        com_init();
        AssignOpr::setMaster(this);
        AssignCallbackFromAgent::setMaster(this);
    }

    void PmVal::com_init(){
        //// to do add to controller
        ctrl->on_pmValue_init(this);
    }

    SliceAgent<PmVal>& PmVal::operator()(int start, int stop){
        auto ret = new SliceAgent<PmVal>(
                    this,
                    getAbsSubSlice(start, stop, getSlice())
            );
        return *ret;
    }

    SliceAgent<PmVal>& PmVal::operator() (int idx){
        return operator() (idx, idx+1);
    }

    SliceAgent<PmVal>& PmVal::operator() (Slice sl){
        return operator() (sl.start, sl.stop);
    }

    Operable* PmVal::doSlice(Slice sl){
        auto& x = operator() (sl.start, sl.stop);
        return x.castToOperable();
    }

    Operable* PmVal::checkShortCircuit() {
        return nullptr;
    }

    void PmVal::createLogicGen(){
        ////// todo create logic gen
        _genEngine = new ParamValGen(_parent->getModuleGen(), this);

    }

}