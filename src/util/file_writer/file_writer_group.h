//
// Created by tanawin on 2/11/2024.
//

#ifndef src_util_fileWriter_FILEWRITERGROUP_H
#define src_util_fileWriter_FILEWRITERGROUP_H

#include "string"
#include "vector"
#include "cassert"
#include "filesystem"

#include "file_writer_base.h"

namespace kathryn{

    //// this class is used to support multiple file writer base
    class FileWriterGroup{
    private:
        std::string _folderPrefix = "/tmp";
        std::vector<FileWriterBase*> file_writers;

    public:

        explicit FileWriterGroup() = default;

        virtual ~FileWriterGroup();

        void    set_prefix_folder(const std::string& folder_prefix);

        void    flush_all();

        void    clean();

        virtual FileWriterBase* create_new_file(const std::string& file_name);



    };

}


#endif //src_util_fileWriter_FILEWRITERGROUP_H
