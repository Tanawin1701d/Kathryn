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

        // if (_master->getGlobalId() == 1701){
        //     std::cout<< "test";
        // }

        if (_master->getMarker() == WMT_INPUT_MD || _master->getMarker() == WMT_OUTPUT_MD){
            UpdatePool& masterUpdatePool = _master->getUpdateMeta();
            assert(masterUpdatePool.size() == 1);

            translatedUpdatePool = masterUpdatePool.clone();
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
            assert(!translatedUpdatePool.isEmpty());
            UpdateEventBase* singleUpdateEvent = translatedUpdatePool.getUpdateEventRef()[0];
            //assert(singleUpdateEvent->getType() == UET_BASIC);

            CbBaseVerilog cb;
            UEBaseGenEngine* ueGenEngine =  singleUpdateEvent->createGenEngine();
            ueGenEngine->genBasicConnect(cb, this);
            delete ueGenEngine;

            return cb.toString(4);
        }

        return assignOpBase();
    }

}