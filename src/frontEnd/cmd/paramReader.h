//
// Created by tanawin on 4/5/2567.
//

#ifndef KATHRYN_PARAMREADER_H
#define KATHRYN_PARAMREADER_H

#include <utility>
#include <sstream>
#include <map>
#include <cassert>

#include "util/fileReader/fileReaderBase.h"

namespace kathryn{

    typedef std::map<std::string, std::string> PARAM;

    class ParamReader: public FileReaderBase{
    public:
        explicit ParamReader(std::string filePath);

         PARAM getKeyVal();


    };


}

#endif //KATHRYN_PARAMREADER_H
