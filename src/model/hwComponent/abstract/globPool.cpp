//
// Created by tanawin on 26/6/2024.
//

#include "globPool.h"

#include <cassert>
#include <vector>
#include <set>
#include "model/hwComponent/abstract/globIo.h"

namespace kathryn{

    ////////// it is used to check how much wire is used
    std::set<std::string> globIoName;
    std::vector<GlobIoItf*> globInputPool;
    std::vector<GlobIoItf*> globOutputPool;
    std::vector<GlobIoItf*> bciPool;

    void addToGlobPool(GlobIoItf* src){
        assert(src != nullptr);
        checkIsThereIoName(src->getGlobIoName());
        addToNameList(src->getGlobIoName());
        if (src->getGlobIoType() == GLOB_IO_INPUT){
            globInputPool.push_back(src); return;
        }
        if (src->getGlobIoType() == GLOB_IO_OUTPUT){
            globOutputPool.push_back(src); return;
        }

        assert(false);
    }

    void addToBciPool(GlobIoItf* src){
        assert(src != nullptr);
        if (src->getGlobIoType() == GLOB_IO_BCI){
            bciPool.push_back(src); return;
        }
        assert(false);

    }

    std::vector<GlobIoItf*>& getGlobPool(bool isInput){
        return isInput ? globInputPool : globOutputPool;
    }
    std::vector<GlobIoItf*>& getBciPool(){
        return bciPool;
    }

    void cleanGlobPool(){
        globInputPool.clear();
        globOutputPool.clear();
    }

    bool checkIsThereIoName(const std::string& test){
        auto iter = globIoName.find(test);
        return iter != globIoName.end();
    }

    void addToNameList(const std::string& ioName){
        globIoName.insert(ioName);
    }


}
