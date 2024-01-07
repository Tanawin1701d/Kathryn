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
    extern std::ofstream* outFile = nullptr;
    ////// MF model formation

    void initDebugger(const std::string& filePath);

    void finalizeDebugger();

    void logMF(const std::string& ident, const std::string& debugMsg);

    void logMD(std::string ident, std::string debugMsg);

}

#endif //KATHRYN_LOGGER_H
