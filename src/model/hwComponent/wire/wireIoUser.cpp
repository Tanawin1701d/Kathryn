//
// Created by tanawin on 1/8/2024.
//

#include "wireIoUser.h"

#include "model/controller/controller.h"
#include "model/hwComponent/module/module.h"


namespace kathryn{


    WireIoUser::WireIoUser(int size, WIRE_IO_TYPE wireIoType):
    Wire(size, false, false),
    _wireIoType(wireIoType){
        com_init();
    }

    void WireIoUser::com_init(){
        ctrl->on_wire_user_io_init(this);
    }

    void WireIoUser::createLogicGen(){
        _genEngine = new WireGen(
            _parent->getModuleGen(),
            this,
            _wireIoType
        );
    }

    bool WireIoUser::verifyWireLocalization(){
        if (_updateMeta.size() != 1){
            return false;
            ///// input or output must be assigned something
        }
        UpdateEvent* ude = _updateMeta[0];
        assert(ude != nullptr);
        if(ude->srcUpdateCondition != nullptr){ return false;}
        if(ude->srcUpdateState != nullptr)    { return false;}
        if(!checkDesIsFullyAssignAndEqual())  { return false;}

        Module* masterModOfThisWire = getParent();
        Module* masterModOfSrcVal = ude->srcUpdateValue->castToIdent()->getParent();
        assert(masterModOfThisWire != nullptr);
        assert(masterModOfSrcVal != nullptr);
        //////// for input wire we must be assign from its master module only
        if (_wireIoType == WIRE_IO_USER_INPUT){
            if (masterModOfThisWire->getParent() != masterModOfSrcVal){
                return false;
            }
        }
        //////// for output wire we must be assign from its inside module or submodule
        if (_wireIoType == WIRE_IO_USER_OUPUT){
            if ( (masterModOfThisWire != masterModOfSrcVal) &&
                 (masterModOfThisWire != masterModOfSrcVal->getParent())){
                return false;
            }
        }

        return true;
    }
}
