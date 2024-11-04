//
// Created by tanawin on 3/11/2024.
//

#ifndef src_sim_modelSimEngine_base_USERDEFREPO_H
#define src_sim_modelSimEngine_base_USERDEFREPO_H


#include<string>
#include<vector>
#include<map>
#include<cassert>
#include "util/fileReader/fileReaderBase.h"


namespace kathryn{


    /*
     * this class is used to track the generated file and collects
     * multiple designers generated piece and replace new one
     */


    /** KATHRYN_DESIGNERS_MANUAL_DEFINE_*/
    constexpr char codeIdenPrefix[] = "KDMD_";
    constexpr int  codeIdenPrefixLength = 5;
    constexpr char codeIdenPrefix_end[] = "KDMD_END";
    constexpr char codeComment[] = "//";
    constexpr int  codeCommentLength = 2;



    struct UserDefRepo{

        //////// key & code;
        std::map<std::string, std::string> repo;

        void addTrackKey(const std::string& key);
        void fetchCode  (FileReaderBase*    fileReader);
        std::string genCode    (const std::string& key);



    };


}

#endif //src_sim_modelSimEngine_base_USERDEFREPO_H
