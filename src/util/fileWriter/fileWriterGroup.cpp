//
// Created by tanawin on 2/11/2024.
//

#include "fileWriterGroup.h"


namespace kathryn{

    FileWriterGroup::~FileWriterGroup(){
        for (auto fileWriter: fileWriters){
            delete fileWriter;
        }
    }

    void
    FileWriterGroup::setPrefixFolder(const std::string& folderPrefix){

        _folderPrefix = folderPrefix;
        assert(std::filesystem::exists(folderPrefix));
    }

    void FileWriterGroup::flushAll(){
        for (FileWriterBase* fileWriter: fileWriters){
            fileWriter->flush();
        }
    }

    void FileWriterGroup::clean(){
        for (FileWriterBase* fileWriter: fileWriters){
            fileWriter->flush();
            delete fileWriter;
        }
        fileWriters.clear();
    }


    FileWriterBase*
    FileWriterGroup::createNewFile(const std::string& fileName){

        fileWriters.emplace_back(
            new FileWriterBase(_folderPrefix+"/"+fileName));
        return fileWriters.back();
    }








}