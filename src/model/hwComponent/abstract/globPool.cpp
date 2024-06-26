//
// Created by tanawin on 26/6/2024.
//

#include "globPool.h"

#include <kathryn.h>


namespace kathryn{

    std::vector<Operable*> globPool;

    void addToGlobPool(Operable* src){
        assert(src != nullptr);
        globPool.push_back(src);
    }

    std::vector<Operable*>& getGlobPool(){
        return globPool;
    }

    void cleanGlobPool(){
        globPool.clear();
    }


}
