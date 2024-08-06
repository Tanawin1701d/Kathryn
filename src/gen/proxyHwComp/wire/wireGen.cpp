//
// Created by tanawin on 4/8/2024.
//

#include "wireGen.h"
#include "model/hwComponent/wire/wire.h"


namespace kathryn{

    /**
     *
     * normal wire
     *
     */
    WireGen::WireGen(ModuleGen* mdGenMaster,
                     Wire*      wireMaster):
    AssignGenBase(mdGenMaster,(Assignable*) wireMaster, (Identifiable*) wireMaster),
    _master(wireMaster)
    { assert(_master != nullptr); }

    void WireGen::routeDep(){

        if (_master->getMarker() == WMT_INPUT_MD || _master->getMarker() == WMT_OUTPUT_MD){
            assert(_master->getUpdateMeta().size() == 1);
            auto* singleUpdateEvent = new UpdateEvent();
            *singleUpdateEvent = *_master->getUpdateMeta()[0];
            translatedUpdateEvent.push_back(singleUpdateEvent);
            return;
        }
        /////// incase normal wire
        AssignGenBase::routeDep();

    }

    std::string WireGen::decIo(){
        assert(_master->getMarker() == WMT_INPUT_MD || _master->getMarker() == WMT_OUTPUT_MD);

        std::string prefix;
        if (_master->getMarker() == WMT_INPUT_MD){
            prefix = "input ";
        }else if (_master->getMarker() == WMT_OUTPUT_MD){
            prefix = "output ";
        }else{
            assert(false);
        }
        Slice sl = _master->getOperableSlice();
        return  prefix + " wire[" + std::to_string(sl.stop-1) +": 0] " + getOpr();

    }
    std::string WireGen::decVariable(){
        Slice sl = _master->getOperableSlice();
        std::string prefixType;
        if(_master->getMarker() == WMT_INPUT_MD || _master->getMarker() == WMT_OUTPUT_MD){
            prefixType = "wire";
        }else{
            prefixType = "reg";
        }
        return prefixType + " [" + std::to_string(sl.stop-1) +": 0] " +getOpr() + ";";
    }
    std::string WireGen::decOp(){

        if(_master->getMarker() == WMT_INPUT_MD || _master->getMarker() == WMT_OUTPUT_MD){
            assert(!translatedUpdateEvent.empty());
            UpdateEvent* singleUpdateEvent = translatedUpdateEvent[0];
            assert(singleUpdateEvent != nullptr);
            return "assign " + getOpr() + " = " +
                getOprStrFromOpr(singleUpdateEvent->srcUpdateValue) + ";";
        }

        return assignOpBase(false);
    }

    bool WireGen::compare(LogicGenBase* lgb){

        assert(lgb->getLogicCef().comptype == HW_COMPONENT_TYPE::TYPE_WIRE);
        auto* rhs = dynamic_cast<WireGen*>(lgb);
        if(_master->getMarker() == WMT_INPUT_MD){
            ///////////// the dep of input wire is master module
            return checkCerfEqLocally(*rhs) && cmpAssignGenBase(rhs, MASTERMOD);
        }
        /////////////// when compare output dep is submodule
        return checkCerfEqLocally(*rhs) && cmpAssignGenBase(rhs, SUBMOD);


    }
}