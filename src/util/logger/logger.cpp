//
// Created by tanawin on 12/12/2566.
//
#include <iostream>
#include <cassert>
#include "logger.h"
#include "model/debugger/modelDebugger.h"


namespace kathryn{


    std::ofstream* outFileMF = nullptr;
    std::ofstream* outFileMD = nullptr;

    std::vector<mFValue> mfStorage;

    std::vector<std::string> mdStorageName;//// key
    std::vector<MdLogVal*> mdStorageVals;

    void initMdDebugger(){
        outFileMD = new std::ofstream("/media/tanawin/tanawin1701e/project2/Kathryn/KOut/MD.txt");
    }

    void initMfDebugger(){

        outFileMF = new std::ofstream("/media/tanawin/tanawin1701e/project2/Kathryn/KOut/MF.txt");

    }

    void printMdLogVal(std::ofstream* mdFile, int ident, MdLogVal* mdLogVal){

        assert(mdFile != nullptr);
        assert(mdLogVal != nullptr);

        for (auto &val: mdLogVal->vals) {
            /** print ident*/
            for (int identCnt = 0; identCnt < ident; identCnt++) {
                *outFileMD << " ";
            }
            *outFileMD << val << "\n";
        }
        ident += 4;
        for (auto subMdLog: mdLogVal->subVal)
            printMdLogVal(mdFile, ident, subMdLog);

    }

    void finalizeMdDebugger(){

        for (int i = 0; i < mdStorageName.size(); i++){
            *outFileMD << "[ " << mdStorageVals[i] << " ]\n";
            printMdLogVal(outFileMD, 0, mdStorageVals[i]);
            *outFileMD << "------------------------------\n";
        }

        outFileMD->close();
        delete outFileMD;
    }

    void flushMfDebugger(){
        for (auto& mfValue : mfStorage){
            *outFileMF << "[ " << mfValue.mdDebug->getMdIdentVal()
                       << " ]    " << mfValue.debugMsg << "\n";
            *outFileMF << "---------------------------\n";
        }
        mfStorage.clear();
        *outFileMF << "---------------------------------------\n";
        *outFileMF << "---------------------------------------\n";
        *outFileMF << "----------------FLUSH------------------\n";
        *outFileMF << "---------------------------------------\n";
        *outFileMF << "---------------------------------------\n";
    }

    void finalizeMfDebugger(){

        outFileMF->close();
        delete outFileMF;
    }

    void logMF(ModelDebuggable* mdDebug,
               const std::string& debugMsg){
        mFValue x = { mdDebug, debugMsg};
        mfStorage.push_back(x);
    }

    void logMD(const std::string& mdName,
               MdLogVal* val){
        assert(val != nullptr);
        mdStorageName.push_back(mdName);
        mdStorageVals.push_back(val);
    }


}