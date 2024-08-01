//
// Created by tanawin on 24/6/2024.
//

#include "wireIo.h"
#include "model/hwComponent/module/module.h"

namespace kathryn{

    WireIoAuto::WireIoAuto(int size, WIRE_IO_TYPE wireIoType):
    Wire(size, false, false),
    _wireIoType(wireIoType){}

    void WireIoAuto::buildHierarchy(Module* masterModule){
        assert(masterModule != nullptr);
        setParent(masterModule);
        buildInheritName();
    }

    void WireIoAuto::createLogicGen(){
        //// assert(_parent->getModuleGen() == nullptr);

        _genEngine = new WireGen(
            _parent->getModuleGen(),
            this,
            _wireIoType
        );
    }


}