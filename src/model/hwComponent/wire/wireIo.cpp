//
// Created by tanawin on 24/6/2024.
//

#include "wireIo.h"

namespace kathryn{

    WireIo::WireIo(int size, WIRE_IO_TYPE wireIoType):
    Wire(size, false, false),
    _wireIoType(wireIoType){}

    void WireIo::buildHierarchy(Module* masterModule){
        assert(masterModule != nullptr);
        setParent(masterModule);
        buildInheritName();
    }


}