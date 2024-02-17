//
// Created by tanawin on 17/2/2567.
//

#ifndef KATHRYN_TYPECONV_H
#define KATHRYN_TYPECONV_H


#include<vector>
#include<stack>
#include <algorithm>
#include <iostream>

namespace kathryn{


    template<typename T>
    std::vector<T> cvtStackToVec(std::stack<T> src){

        std::vector<T> result;
        while(!src.empty()){
            result.push_back(src.top());
            src.pop();
        }
        for (int i = 0; i < (result.size()/2); i++){
            std::swap(result[i], result[result.size()-1-i]);
        }
        return result;

    }

}

#endif //KATHRYN_TYPECONV_H
