//
// Created by tanawin on 12/12/2566.
//

#ifndef KATHRYN_LOGGER_H
#define KATHRYN_LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace kathryn{

    /*** In the future we will build debug model more efficiently*/

    class ModelDebuggable;

    struct mFValue{
        ModelDebuggable* mdDebug;
        std::string      debugMsg;
    };

    struct MdLogVal{
        std::vector<std::string> vals;
        std::vector<MdLogVal*>   subVal;

        void addVal(const std::string& val){ vals.push_back(val); }

        MdLogVal* makeNewSubVal(){
            auto sub = new MdLogVal();
            subVal.push_back(sub);
            return sub;
        }
    };

    extern std::string outPath;
    extern std::vector<mFValue> mfStorage;
    extern std::vector<MdLogVal*> mdStorageVals;
    extern std::vector<std::string> mdStorageName;//// key
    /////extern std::ofstream* outFileMF;
    ////// MF model formation

    void initMdDebugger();
    void initMfDebugger();

    void finalizeMdDebugger();

    void flushMfDebugger();
    void finalizeMfDebugger();


    void logMF(ModelDebuggable* mdDebug, const std::string& debugMsg);

    void logMD(const std::string& mdName, MdLogVal* val);


}

#endif //KATHRYN_LOGGER_H
