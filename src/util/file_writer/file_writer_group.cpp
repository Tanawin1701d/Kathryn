//
// Created by tanawin on 2/11/2024.
//

#include "file_writer_group.h"


namespace kathryn{

    FileWriterGroup::~FileWriterGroup(){
        for (auto file_writer: file_writers){
            delete file_writer;
        }
    }

    void
    FileWriterGroup::set_prefix_folder(const std::string& folder_prefix){

        _folderPrefix = folder_prefix;
        assert(std::filesystem::exists(folder_prefix));
    }

    void FileWriterGroup::flush_all(){
        for (FileWriterBase* file_writer: file_writers){
            file_writer->flush();
        }
    }

    void FileWriterGroup::clean(){
        for (FileWriterBase* file_writer: file_writers){
            file_writer->flush();
            delete file_writer;
        }
        file_writers.clear();
    }


    FileWriterBase*
    FileWriterGroup::create_new_file(const std::string& file_name){

        file_writers.emplace_back(
            new FileWriterBase(_folderPrefix+"/"+file_name));
        return file_writers.back();
    }








}