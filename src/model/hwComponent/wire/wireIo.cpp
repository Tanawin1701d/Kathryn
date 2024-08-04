//
// Created by tanawin on 24/6/2024.
//

#include "wireIo.h"
#include "model/hwComponent/module/module.h"
#include "model/controller/controller.h"


namespace kathryn{

    WireIo::WireIo(int size, WIRE_IO_TYPE wireIoType):
    Wire(size, false, false),
    _wireIoType(wireIoType){

        if ( (_wireIoType == WIRE_IO_USER_INPUT) ||
             (_wireIoType == WIRE_IO_USER_OUPUT)
        ){
            com_init();
        }

    }

    void WireIo::com_init(){
        ctrl->on_wire_user_io_init(this);
    }

    void WireIo::buildHierarchy(Module* masterModule){
        assert(masterModule != nullptr);
        setParent(masterModule);
        buildInheritName();
    }

    void WireIo::createLogicGen(){
        //// assert(_parent->getModuleGen() == nullptr);

        _genEngine = new WireGen(
            _parent->getModuleGen(),
            this,
            _wireIoType
        );
    }

    bool WireIo::verifyInternalWireLocalization(){
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