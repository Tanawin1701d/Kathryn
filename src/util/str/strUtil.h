//
// Created by tanawin on 17/2/2567.
//

#ifndef KATHRYN_STRUTIL_H
#define KATHRYN_STRUTIL_H


#include <string>
#include <vector>
#include <sstream>

namespace kathryn{

    std::string genConString(char ch, int amt);

    std::vector<std::string> splitStr(const std::string& str, char dilem);
    std::string joinStr(const std::vector<std::string>& strs, char dilem);



}

#endif //KATHRYN_STRUTIL_H
