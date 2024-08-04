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
    std::vector<WireMarker*>  globInputPool;
    std::vector<WireMarker*>  globOutputPool;
    std::vector<WireMarker*>  mdInputPool;
    std::vector<WireMarker*>  mdOutputPool;


    void addToGlobPool(WireMarker* src){
        assert(src != nullptr);
        checkIsThereIoName(src->getGlobIoName());
        addToNameList(src->getGlobIoName());
        if (src->getGlobIoType() == WMT_GLOB_INPUT){
            globInputPool.push_back(src); return;
        }
        if (src->getGlobIoType() == WMT_GLOB_OUTPUT){
            globOutputPool.push_back(src); return;
        }
        if (src->getGlobIoType() == WMT_INPUT_MD){
            mdInputPool.push_back(src); return;
        }
        if (src->getGlobIoType() == WMT_OUTPUT_MD){
            mdOutputPool.push_back(src); return;
        }
        assert(false);
    }

    std::vector<WireMarker*>& getGlobPool(bool isInput){
        return isInput ? globInputPool: globOutputPool;
    }

    std::vector<WireMarker*>& getMdIoPool(bool isInput){
        return isInput ? mdInputPool: mdOutputPool;
    }

    void cleanGlobPool(){
        globIoName    .clear();
        globInputPool .clear();
        globOutputPool.clear();
        mdInputPool   .clear();
        mdOutputPool  .clear();
    }

    bool checkIsThereIoName(const std::string& test){
        auto iter = globIoName.find(test);
        return iter != globIoName.end();
    }

    void addToNameList(const std::string& ioName){
        globIoName.insert(ioName);
    }


}
