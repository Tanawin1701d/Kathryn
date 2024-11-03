//
// Created by tanawin on 2/11/2024.
//

#ifndef src_util_fileWriter_FILEWRITERGROUP_H
#define src_util_fileWriter_FILEWRITERGROUP_H

#include<string>
#include <vector>
#include<cassert>
#include<filesystem>

#include "fileWriterBase.h"

namespace kathryn{

    //// this class is used to support multiple file writer base
    class FileWriterGroup{
    private:
        std::string _folderPrefix = "/tmp";
        std::vector<FileWriterBase*> fileWriters;

    public:

        explicit FileWriterGroup() = default;

        virtual ~FileWriterGroup();

        void    setPrefixFolder(const std::string& folderPrefix);

        void    flushAll();

        void    clean();

        virtual FileWriterBase* createNewFile(const std::string& fileName);



    };

}


#endif //src_util_fileWriter_FILEWRITERGROUP_H
