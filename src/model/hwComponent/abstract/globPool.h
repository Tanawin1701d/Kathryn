//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBPOOL_H
#define GLOBPOOL_H

#include "globIo.h"
#include "operable.h"

namespace kathryn{


    void addToGlobPool(WireMarker* src);
    std::vector<WireMarker*>& getGlobPool(bool isInput);
    std::vector<WireMarker*>& getMdIoPool(bool isInput);

    void cleanGlobPool();

    bool checkIsThereIoName(const std::string& test);
    void addToNameList(const std::string& ioName);

}

#endif //GLOBPOOL_H
