//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_LOGGER_H
#define KATHRYN_LOGGER_H

#include <iostream>
#include <fstream>
#include <string>

namespace kathryn{

    /*** In the future we will build debug model more efficiently*/
    extern std::string outPath;
    extern std::ofstream* outFile;
    ////// MF model formation

    void initDebugger();

    void finalizeDebugger();

    void logMF(const std::string& ident, const std::string& debugMsg);

    void logMD(const std::string& ident, const std::string& debugMsg);

}

#endif //KATHRYN_LOGGER_H
