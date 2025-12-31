//
// Created by tanawin on 17/2/2567.
//
#include"strUtil.h"

namespace kathryn{


    std::string genConString(char ch, int amt){
        std::string result;
        for (int i = 0; i < amt; i++){
            result += ch;
        }
        return result;
    }


    std::vector<std::string> splitStr(const std::string& str, char delim) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string item;
        while (std::getline(ss, item, delim)) {
            tokens.push_back(item);
        }
        return tokens;
    }

    std::string joinStr(const std::vector<std::string>& strs, char dilem){

        if (strs.empty()){return "";}

        std::string result;
        for (const auto& data: strs){
                    result += data + dilem;
        }
        ///// remove last dilema
        result.erase(strs.size()-1);
        return result;
    }


}