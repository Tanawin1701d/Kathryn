//
// Created by tanawin on 24/6/2024.
//

#include "wireIo.h"

namespace kathryn{

    WireIo::WireIo(int size, bool isInput):
    Wire(size, false, false),
    _isInput(isInput){}

    void WireIo::buildHierarchy(Module* masterModule){
        assert(masterModule != nullptr);
        setParent(masterModule);
        buildInheritName();
    }


}