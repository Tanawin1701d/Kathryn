//
// Created by tanawin on 24/6/2024.
//

#include "gen/proxyHwComp/wire/wireAutoGen.h"
#include "model/hwComponent/module/module.h"

namespace kathryn{

    WireAuto::WireAuto(int size, WIRE_AUTO_GEN_TYPE wireIoType):
    Wire(size, false, false),
    _wireIoType(wireIoType){}

    void WireAuto::buildHierarchy(Module* masterModule){
        assert(masterModule != nullptr);
        setParent(masterModule);
        buildInheritName();
    }

    void WireAuto::createLogicGen(){
        //// assert(_parent->getModuleGen() == nullptr);

        _genEngine = new WireAutoGen(
            _parent->getModuleGen(),
            this,
            _wireIoType
        );
    }


}