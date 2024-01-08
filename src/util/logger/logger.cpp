//
// Created by tanawin on 12/12/2566.
//
#include <iostream>
#include "logger.h"


namespace kathryn{

    std::string outPath = "/media/tanawin/tanawin1701e/project2/Kathryn/KOut/test.txt";
    std::ofstream* outFile = nullptr;

    void initDebugger(){
        outFile = new std::ofstream(outPath);
    }

    void finalizeDebugger(){
        outFile->close();
        delete outFile;
    }

    std::string lastMF_ident = "$";

    void logMF(const std::string& ident,
               const std::string& debugMsg){

        if (outFile == nullptr){
            initDebugger();
        }

        if (lastMF_ident != ident){
            *outFile << "----------------------------------\n";
            lastMF_ident = ident;
        }
        *outFile << "[ "<< ident << " ] " << debugMsg << "\n";

    }

    std::string lastMD_ident = "$";

    void logMD(const std::string& ident,
               const std::string& debugMsg){

        if (outFile == nullptr){
            initDebugger();
        }

        if (lastMF_ident != ident){
            *outFile << "----------------------------------\n";
            lastMD_ident = ident;
        }
        *outFile << "[ "<< ident << " ] " << debugMsg << "\n";

    }


}