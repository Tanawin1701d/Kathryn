//
// Created by tanawin on 3/11/2024.
//

#ifndef src_sim_modelSimEngine_base_USERDEFREPO_H
#define src_sim_modelSimEngine_base_USERDEFREPO_H


#include "string"
#include "vector"
#include "map"
#include "cassert"
#include "util/file_reader/file_reader_base.h"


namespace kathryn{


    /*
     * this class is used to track the generated file and collects
     * multiple designers generated piece and replace new one
     */


    /** KATHRYN_DESIGNERS_MANUAL_DEFINE_*/
    constexpr char code_iden_prefix[] = "KDMD_";
    constexpr int  code_iden_prefix_length = 5;
    constexpr char codeIdenPrefix_end[] = "KDMD_END";
    constexpr char code_comment[] = "//";
    constexpr int  code_comment_length = 2;



    struct UserDefRepo{

        //////// key & code;
        std::map<std::string, std::string> repo;

        void add_track_key(const std::string& key);
        void fetch_code  (FileReaderBase*    file_reader);
        std::string gen_code    (const std::string& key);



    };


}

#endif //src_sim_modelSimEngine_base_USERDEFREPO_H
