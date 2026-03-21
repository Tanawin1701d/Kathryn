//
// Created by tanawin on 17/2/2567.
//

#ifndef KATHRYN_STRUTIL_H
#define KATHRYN_STRUTIL_H


#include "string"
#include "vector"
#include "sstream"

namespace kathryn{

    std::string gen_con_string(char ch, int amt);

    std::vector<std::string> split_str(const std::string& str, char dilem);
    std::string join_str(const std::vector<std::string>& strs, char dilem);



}

#endif //KATHRYN_STRUTIL_H
