//
// Created by tanawin on 3/11/2024.
//

#include "user_def_repo.h"


namespace kathryn{


    void UserDefRepo::add_track_key(const std::string& key){
        repo.insert({key, ""});
    }

    void UserDefRepo::fetch_code(FileReaderBase* file_reader){
        assert(file_reader != nullptr);

        while (file_reader->is_next()){
            /*find prefix*/
            std::string line = file_reader->readline();
            //std::cout << "line read X      " << line <<std::endl;
            size_t prefix_pos = line.find(code_iden_prefix);
            if (prefix_pos == std::string::npos){continue;}

            std::string key;
            std::string result;

            /** retrieve key*/
            key = line.substr(prefix_pos + code_iden_prefix_length);

            /**retrieve code and check for the end*/;
            while (file_reader->is_next()){
                line = file_reader->readline();
                //std::cout << "line read Y       " << line <<std::endl;
                if (line.find(codeIdenPrefix_end) == std::string::npos){
                    result += line + "\n";
                }else{
                    //////// put code to the repo
                    if (repo.find(key) != repo.end()){
                        repo[key] = result;
                    }else{
                        std::cout << "there is no key " << key << std::endl;
                        assert(false);
                    }
                    break;
                }
            }
        }
    }

    std::string UserDefRepo::gen_code(const std::string& key){

        auto find_iter = repo.find(key);
        assert(find_iter != repo.end());

        /**retrieve code*/
        std::string ret = std::string(code_comment) + code_iden_prefix + key + "\n";
        std::string code = find_iter->second;
        if (code.empty()){
            ret += "\n\n\n " + std::string(code_comment) +
                   "you may put the code between "
                   "KDMD_ and KDMD_EN D the code will be not deleted"
                   "\n\n\n";
        }else{
            ret += code;
        }
        ret += std::string(code_comment) + codeIdenPrefix_end;
        return ret;
    }




}