//
// Created by tanawin on 3/11/2024.
//

#include "userDefRepo.h"


namespace kathryn{


    void UserDefRepo::addTrackKey(const std::string& key){
        repo.insert({key, ""});
    }

    void UserDefRepo::fetchCode(FileReaderBase* fileReader){
        assert(fileReader != nullptr);

        while (fileReader->isNext()){
            /*find prefix*/
            std::string line = fileReader->readline();
            //std::cout << "line read X      " << line <<std::endl;
            size_t prefixPos = line.find(codeIdenPrefix);
            if (prefixPos == std::string::npos){continue;}

            std::string key;
            std::string result;

            /** retrieve key*/
            key = line.substr(prefixPos + codeIdenPrefixLength);

            /**retrieve code and check for the end*/;
            while (fileReader->isNext()){
                line = fileReader->readline();
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

    std::string UserDefRepo::genCode(const std::string& key){

        auto findIter = repo.find(key);
        assert(findIter != repo.end());

        /**retrieve code*/
        std::string ret = std::string(codeComment) + codeIdenPrefix + key + "\n";
        std::string code = findIter->second;
        if (code.empty()){
            ret += "\n\n\n " + std::string(codeComment) +
                   "you may put the code between "
                   "KDMD_ and KDMD_EN D the code will be not deleted"
                   "\n\n\n";
        }else{
            ret += code;
        }
        ret += std::string(codeComment) + codeIdenPrefix_end;
        return ret;
    }




}