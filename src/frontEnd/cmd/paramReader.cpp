//
// Created by tanawin on 4/5/2567.
//


#include "paramReader.h"
#include "model/debugger/modelDebugger.h"


namespace kathryn{


    ParamReader::ParamReader(std::string filePath) :
    FileReaderBase(std::move(filePath)){}

    std::map<std::string, std::string> ParamReader::getKeyVal() {

        std::vector<std::string> rawLines = readLines();
        std::map<std::string, std::string> result;

        /** traverse line by line*/
        for (std::string& line : rawLines){
            /***split*/
            std::istringstream buffer(line);
            std::vector<std::string> splited;
            std::string token;
            while (buffer >> token){
                splited.push_back(token);
            }
            /** add value to storage*/
            if (splited.empty() || (splited[0] == ";")){
                continue;
            }else if(splited.size() == 3){
                assert(splited[1] == "=");
                mfAssert(result.find(splited[0]) == result.end(),
                         "detect duplicate parameter"
                         );
                result[splited[0]] = splited[2];
            }else{
                mfAssert(false, "invalid paraminput");
            }
        }

        return result;

    }
}