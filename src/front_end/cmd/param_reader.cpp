//
// Created by tanawin on 4/5/2567.
//


#include "param_reader.h"
#include "model/debugger/model_debugger.h"


namespace kathryn{


    ParamReader::ParamReader(std::string file_path) :
    FileReaderBase(std::move(file_path)){}

    std::map<std::string, std::string> ParamReader::get_key_val() {

        std::vector<std::string> raw_lines = read_lines();
        std::map<std::string, std::string> result;

        /** traverse line by line*/
        for (std::string& line : raw_lines){
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
                mf_assert(result.find(splited[0]) == result.end(),
                         "detect duplicate parameter"
                         );
                result[splited[0]] = splited[2];
            }else{
                mf_assert(false, "invalid paraminput");
            }
        }

        return result;

    }
}