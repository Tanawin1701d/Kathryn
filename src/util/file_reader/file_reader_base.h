//
// Created by tanawin on 4/5/2567.
//

#ifndef KATHRYN_FILEREADERBASE_H
#define KATHRYN_FILEREADERBASE_H

#include "iostream"
#include "fstream"
#include "cstring"
#include "vector"

namespace kathryn{

    class FileReaderBase{
    private:
        std::string _fileName;
        std::ifstream _inputFile;

    public:
        explicit FileReaderBase(std::string file_path);
        virtual ~FileReaderBase() = default;
        std::vector<std::string> read_lines();
        /////// for next line
        bool        is_next();
        std::string readline();


    };



}

#endif //KATHRYN_FILEREADERBASE_H
