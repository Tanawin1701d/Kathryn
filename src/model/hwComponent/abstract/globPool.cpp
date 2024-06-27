//
// Created by tanawin on 26/6/2024.
//

#include <cassert>
#include <vector>
#include "model/hwComponent/abstract/globIo.h"

namespace kathryn{

    std::vector<GlobIo*> globInputPool;
    std::vector<GlobIo*> globOutputPool;

    void addToGlobPool(GlobIo* src){
        assert(src != nullptr);
        if (src->getGlobIoType() == GLOB_IO_INPUT){
            globInputPool.push_back(src); return;
        }
        if (src->getGlobIoType() == GLOB_IO_OUTPUT){
            globOutputPool.push_back(src); return;
        }
        assert(false);
    }

    std::vector<GlobIo*>& getGlobPool(bool isInput){
        return isInput ? globInputPool : globOutputPool;
    }

    void cleanGlobPool(){
        globInputPool.clear();
        globOutputPool.clear();
    }


}
