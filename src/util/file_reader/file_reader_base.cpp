//
// Created by tanawin on 4/5/2567.
//

#include "cassert"
#include "file_reader_base.h"


namespace kathryn{


    FileReaderBase::FileReaderBase(std::string file_path):
    _fileName(file_path),
    _inputFile(file_path)
    {
        assert(_inputFile.is_open());
    }



    std::vector<std::string> FileReaderBase::read_lines() {

        std::string line;
        std::vector<std::string> results;
        /*** read file */
        while(std::getline(_inputFile, line)){
            results.push_back(line);
        }

        return results;
    }

    bool FileReaderBase::is_next(){
        return _inputFile.peek() != EOF;
    }

    std::string FileReaderBase::readline(){
        std::string line;
        std::getline(_inputFile, line);
        return line;
    }




}