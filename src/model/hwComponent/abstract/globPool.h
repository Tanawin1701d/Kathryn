//
// Created by tanawin on 26/6/2024.
//

#ifndef GLOBPOOL_H
#define GLOBPOOL_H

#include "globIo.h"
#include "operable.h"

namespace kathryn{


    void addToGlobPool(GlobIoItf* src);
    void addToBciPool(GlobIoItf* src);
    std::vector<GlobIoItf*>& getGlobPool(bool isInput);
    void cleanGlobPool();

    bool checkIsThereIoName(const std::string& test);
    void addToNameList(const std::string& ioName);

}

#endif //GLOBPOOL_H
