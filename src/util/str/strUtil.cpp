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


}