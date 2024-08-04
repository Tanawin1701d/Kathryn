//
// Created by tanawin on 24/6/2024.
//

#include "wireAutoGen.h"
#include "model/hwComponent/module/module.h"

namespace kathryn{

    WireAutoGen::WireAutoGen(int size, WIRE_AUTO_GEN_TYPE wireIoType):
    Wire(size, false, false),
    _wireIoType(wireIoType){}

    void WireAutoGen::buildHierarchy(Module* masterModule){
        assert(masterModule != nullptr);
        setParent(masterModule);
        buildInheritName();
    }

    void WireAutoGen::createLogicGen(){
        //// assert(_parent->getModuleGen() == nullptr);

        _genEngine = new WireGen(
            _parent->getModuleGen(),
            this,
            _wireIoType
        );
    }


}